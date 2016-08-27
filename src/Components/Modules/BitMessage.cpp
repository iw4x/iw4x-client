#include "STDInclude.hpp"

#ifndef DISABLE_BITMESSAGE

#include <Shlwapi.h>

using namespace Utils;

namespace Components
{
	BitMessage* BitMessage::Singleton = nullptr;

	BitMessage::BitMessage()
	{
		if (Singleton != nullptr)
		{
			throw new std::runtime_error("Only 1 BitMessage instance allowed at the same time.");
		}
		Singleton = this;

		Logger::Print("Initializing BitMessage...\n");
		this->BMClient = new BitMRC(BITMESSAGE_OBJECT_STORAGE_FILENAME, BITMESSAGE_KEYS_FILENAME);
		this->BMClient->init();
		this->BMClient->defaultTTL = 1 * 60 * 60; // 1 hour

		if (this->BMClient->PrivAddresses.empty())
		{
			if (!this->InitAddr())
			{
				// Generate a random address ready to use
				throw std::runtime_error("Failed to prepare source address for exception handling");
			}
			this->BMClient->save();
		}

		this->BMClient->start();

		Command::Add("bm_send", [](Command::Params params) {
			if (params.Length() < 3) return;

			ustring pubAddrString;
			pubAddrString.fromString(params[1]);
			PubAddr pubAddr;
			if (pubAddr.loadAddr(pubAddrString))
			{
				ustring msg;
				msg.fromString(params.Join(2));

				Logger::Print("Sending message (this may take a while)...\n");
				Singleton->BMClient->sendMessage(msg, pubAddr, Singleton->BMClient->PrivAddresses[0]);
				Logger::Print("Message sent.\n");
			}
			else
			{
				Logger::Print("Address not correct!\n");
			}
		});
		Command::Add("bm_sendb", [](Command::Params params) {
			if (params.Length() < 2) return;

			ustring msg;
			msg.appendVarString(params.Join(1));
			Logger::Print("Sending broadcast...\n");
			Singleton->BMClient->sendBroadcast(msg, Singleton->BMClient->PrivAddresses[0]);
			Logger::Print("Broadcast done.\n");
		});
		Command::Add("bm_check_messages", [](Command::Params) {
			while (Singleton->BMClient->new_messages.size() > 0)
			{
				auto msg = Singleton->BMClient->new_messages.pop();
				Logger::Print("New message:\nFrom: %s\nTo: %s\nMessage:\n%s\n", msg.from.c_str(), msg.to.c_str(), msg.info.c_str());
			}
		});
		Command::Add("bm_check_connections", [](Command::Params) {
			std::shared_lock<std::shared_timed_mutex> mlock(Singleton->BMClient->mutex_nodes);

			for (auto& node : Singleton->BMClient->Nodes)
			{
				switch (node->state) {
				case 0: // Not connected
					Logger::Print("%s: Disconnected\n", node->Ip.c_str());
					break;
				case 1: // Connecting
					Logger::Print("%s: Connecting\n", node->Ip.c_str());
					break;
				case 2: // Connected
					Logger::Print("%s: Connected\n", node->Ip.c_str());
					break;
				case 3: // Reconnecting
					Logger::Print("%s: Reconnecting\n", node->Ip.c_str());
					break;
				}
			}

			mlock.unlock();
		});
		Command::Add("bm_check_privatekey", [](Command::Params) {
			std::shared_lock<std::shared_timed_mutex> mlock(Singleton->BMClient->mutex_priv);

			if (Singleton->BMClient->PrivAddresses.empty()) {
				Logger::Print("No private key\n");
			}
			else
				for (auto& addr : Singleton->BMClient->PrivAddresses)
				{
					Logger::Print("%s\n", addr.getAddress().c_str());
				}

			mlock.unlock();
		});
		Command::Add("bm_check_publickey", [](Command::Params) {
			std::shared_lock<std::shared_timed_mutex> mlock(Singleton->BMClient->mutex_pub);

			if (Singleton->BMClient->PubAddresses.empty()) {
				Logger::Print("No public key\n");
			}
			else
				for (auto& addr : Singleton->BMClient->PubAddresses)
				{
					Logger::Print("%s\n", addr.getAddress().c_str());
				}

			mlock.unlock();
		});
		Command::Add("bm_save", [](Command::Params) {
			Singleton->BMClient->save(BITMESSAGE_OBJECT_STORAGE_FILENAME);
		});
		Command::Add("bm_address_public", [](Command::Params params) {
			if (params.Length() < 2) return;

			ustring addre;
			addre.fromString(params.Join(1));
			PubAddr address;
			if (address.loadAddr(addre))
			{
				Logger::Print("Asking public key!\n");
				Singleton->BMClient->getPubKey(address);
				Logger::Print("Asked! check publickey for news on that address!\n");
			}
			else
			{
				Logger::Print("Address not correct!\n");
			}
		});
		Command::Add("bm_address_broadcast", [](Command::Params params) {
			if (params.Length() < 2) return;

			ustring addre;
			addre.fromString(params.Join(1));
			PubAddr address;
			if (address.loadAddr(addre))
			{
				Logger::Print("Adding subscription!\n");
				Singleton->BMClient->addSubscription(address);
			}
			else
			{
				Logger::Print("Address not correct!\n");
			}
		});
	}

	bool BitMessage::InitAddr()
	{
		Logger::Print("Generating BM address...\n");
		Addr myAddress;
		if (!myAddress.generateRandom())
		{
			return false;
		}
		BMClient->addAddr(myAddress);
		return true;
	}
}

#endif