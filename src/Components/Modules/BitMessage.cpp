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
		this->LoadObjectsAndKeys();
		this->LoadNodes();

		if (this->BMClient->PrivAddresses.empty())
		{
			if (!this->InitAddr())
			{
				// Generate a random address ready to use
				throw std::runtime_error("Failed to prepare source address for exception handling");
			}
			this->SaveObjectsAndKeys();
		}

		this->BMClient->start();

		Command::Add("bm_sendb", [](Command::Params) {
			ustring msg;
			msg.appendVarString("testing");
			Singleton->BMClient->sendBroadcast(msg, Singleton->BMClient->PrivAddresses[0]);
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
		Command::Add("bm_save_nodes", [](Command::Params) {
			Singleton->SaveNodes();
		});
		Command::Add("bm_address_public", [](Command::Params params) {
			if (params.Length() < 1) return;

			ustring addre;
			addre.fromString(params[0]);
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
			if (params.Length() < 1) return;

			ustring addre;
			addre.fromString(params[0]);
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

	void BitMessage::LoadObjectsAndKeys()
	{
		Logger::Print("Loading BM objects and keys...\n");
		this->BMClient->load(BITMESSAGE_OBJECT_STORAGE_FILENAME);
	}

	void BitMessage::SaveObjectsAndKeys()
	{
		Logger::Print("Saving BM objects and keys...\n");
		this->BMClient->save(BITMESSAGE_OBJECT_STORAGE_FILENAME);
	}

	void BitMessage::SaveNodes()
	{
		Logger::Print("Saving BM nodes...\n");
		Proto::Node::List list;
		std::vector<std::string> nodeList; // keep track of dupes

		std::shared_lock<std::shared_timed_mutex> mlock(Singleton->BMClient->mutex_nodes);

		for (auto& node : Singleton->BMClient->Nodes)
		{
			std::string nodeStr = String::VA("%s:%s", node->Ip.c_str(), node->Port.c_str());
			switch (node->state) {
			case 0: // Not connected
				break;
			case 1: // Connecting
				break;
			case 2: // Connected
				if (std::find(nodeList.begin(), nodeList.end(), nodeStr) == nodeList.end())
				{
					// This allows us to keep track of duplicates
					nodeList.push_back(nodeStr);

					DWORD ip = 0;
					unsigned short port = 0;
					sscanf_s(nodeStr.c_str(), "%d.%d.%d.%d:%hu", &ip, &ip + 1, &ip + 2, &ip + 3, &port);

					Network::Address address;
					address.SetIP(ip);
					address.SetPort(port);

					address.Serialize(list.add_address());

					Logger::Print("Saved node: %s\n", address.GetCString());
				}
				break;
			case 3: // Reconnecting
				break;
			}
		}

		mlock.unlock();

		if (list.address_size() <= 0)
			return;

		// Create directory if it doesn't exist yet
		auto dirName = const_cast<char*>(BITMESSAGE_NODE_STORAGE_FILENAME.c_str());
		PathRemoveFileSpecA(dirName);
		CreateDirectoryA(dirName, NULL);

		// Now write the actual file
		Utils::IO::WriteFile(BITMESSAGE_NODE_STORAGE_FILENAME, list.SerializeAsString());
	}

	void BitMessage::LoadNodes()
	{
		Logger::Print("Loading BM nodes...\n");
		// Load nodes stored in file
		Proto::Node::List list;
		std::string nodes = IO::ReadFile(BITMESSAGE_NODE_STORAGE_FILENAME);
		if (!nodes.empty() && list.ParseFromString(nodes))
		{
			for (int i = 0; i < list.address_size(); ++i)
			{
				Network::Address address = list.address(i);

				this->BMClient->connectNode(new NodeConnection(
					String::VA("%d.%d.%d.%d", address.GetIP().bytes[0], address.GetIP().bytes[1], address.GetIP().bytes[2], address.GetIP().bytes[3]), // urgh.
					String::VA("%d", address.GetPort()), this->BMClient));
			}
		}

		// Load default nodes if not connected to yet
		for (auto addr : bitmessageKnownNodes) {
			auto portStr = std::string(String::VA("%d", addr.second));
			if (!this->HasNode(addr.first, portStr))
			{
				this->BMClient->connectNode(new NodeConnection(addr.first, portStr, this->BMClient));
			}
		}
	}

	bool BitMessage::HasNode(std::string ip, std::string port)
	{
		for (auto& node : this->BMClient->Nodes)
		{
			if (node->Ip == ip && node->Port == port)
			{
				return true;
			}
		}

		return false;
	}
}

#endif