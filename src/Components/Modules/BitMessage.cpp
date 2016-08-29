#include "STDInclude.hpp"

#ifndef DISABLE_BITMESSAGE

#include <Shlwapi.h>

using namespace Utils;

namespace Components
{
	BitMessage* BitMessage::Singleton = NULL;

	BitMessage::BitMessage()
	{
		if (Singleton != NULL)
			throw new std::runtime_error("Only one instance of BitMessage allowed at the same time.");

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
					Logger::Print("%s (waiting for public key: %s)\n", addr.getAddress().c_str(), addr.waitingPubKey() ? "yes" : "no");
				}

			mlock.unlock();
		});
		Command::Add("bm_save", [](Command::Params) {
			Singleton->Save();
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

	BitMessage::~BitMessage()
	{
		Singleton = NULL;
		delete this->BMClient;
	}

	void BitMessage::SetDefaultTTL(time_t ttl)
	{
		this->BMClient->defaultTTL = ttl;
	}

	bool BitMessage::RequestPublicKey(std::string targetAddress)
	{
		// Convert to ustring
		ustring targetAddressU;
		targetAddressU.fromString(targetAddress);

		// Convert to PubAddr
		PubAddr pubAddr;
		if (!pubAddr.loadAddr(targetAddressU))
		{
			return false;
		}

		// Request public key!
		this->BMClient->getPubKey(pubAddr);
		return true;
	}

	PubAddr* BitMessage::FindPublicKey(PubAddr address)
	{
		std::shared_lock<std::shared_timed_mutex> mlock(Singleton->BMClient->mutex_pub);

		PubAddr* retval = nullptr;

		for (auto& pubKey : BMClient->PubAddresses)
		{
			if (pubKey.getVersion() == address.getVersion()) //check same version
			{
				if ((address.getVersion() >= 4 && pubKey.getTag() == address.getTag()) // version 4+ equality check
					|| (pubKey.getRipe() == address.getRipe())) // version 3- equality check
				{
					retval = &pubKey;
					break;
				}
			}
		}
		mlock.unlock();
		return retval;
	}

	bool BitMessage::WaitForPublicKey(std::string targetAddress)
	{
		// Convert to ustring
		ustring targetAddressU;
		targetAddressU.fromString(targetAddress);

		// Convert to PubAddr
		PubAddr address;
		if (!address.loadAddr(targetAddressU))
		{
			return false;
		}

		// Resolve our own copy to the registered PubAddr copy in BitMRC if possible
		auto resolvedAddress = FindPublicKey(address);
		if (resolvedAddress != nullptr &&
			!resolvedAddress->waitingPubKey() && !resolvedAddress->getPubEncryptionKey().empty())
			return true;

		if (resolvedAddress == nullptr ||
			(!resolvedAddress->waitingPubKey() && resolvedAddress->getPubEncryptionKey().empty()))
		{
			// Request public key
			this->BMClient->getPubKey(address);
			resolvedAddress = FindPublicKey(address);
		}
		this->Save();

		// TODO: Wait for public key by using signaling in BitMRC, needs to be done directly in the fork.
		while (resolvedAddress->waitingPubKey())
		{
			sleep(1500);
		}
		this->Save();

		return true;
	}

	bool BitMessage::Subscribe(std::string targetAddress)
	{
		// Convert to ustring
		ustring targetAddressU;
		targetAddressU.fromString(targetAddress);

		// Convert to PubAddr
		PubAddr pubAddr;
		if (!pubAddr.loadAddr(targetAddressU))
		{
			return false;
		}

		// Subscribe!
		this->BMClient->addSubscription(pubAddr);
		return true;
	}

	bool BitMessage::SendMsg(std::string targetAddress, std::string message, time_t ttl)
	{
		// Convert target address to ustring
		ustring targetAddressU;
		targetAddressU.fromString(targetAddress);

		// Convert target address to PubAddr
		PubAddr pubAddr;
		if (!pubAddr.loadAddr(targetAddressU))
		{
			return false;
		}

		// Convert message to ustring
		ustring messageU;
		messageU.fromString(message);

		// Send the message
		// TODO - Set mutex on priv when accessing first private address
		if (ttl > 0)
			this->BMClient->sendMessage(messageU, pubAddr, this->BMClient->PrivAddresses[0], ttl);
		else
			this->BMClient->sendMessage(messageU, pubAddr, this->BMClient->PrivAddresses[0]);
		return true;
	}

	bool BitMessage::SendBroadcast(std::string message, time_t ttl)
	{
		// Convert message to ustring
		ustring messageU;
		messageU.fromString(message);

		// TODO - Set mutex on priv when accessing first private address
		if (ttl > 0)
			this->BMClient->sendBroadcast(messageU, this->BMClient->PrivAddresses[0], ttl);
		else
			this->BMClient->sendBroadcast(messageU, this->BMClient->PrivAddresses[0]);
		return true;
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

	void BitMessage::Save()
	{
		BMClient->save();
	}
}

#endif