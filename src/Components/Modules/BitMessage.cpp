#include "STDInclude.hpp"

#ifndef DISABLE_BITMESSAGE

#include <Shlwapi.h>

using namespace Utils;

namespace Components
{
	std::thread BitMessage::ShutDownThread;
	BitMRC* BitMessage::BMClient;

	BitMessage::BitMessage()
	{
#ifdef DEBUG
		Logger::Print("Initializing BitMessage...\n");
#endif

		QuickPatch::OnShutdown([] ()
		{
			BitMessage::ShutDownThread = std::thread(BitMessage::ShutDown);
			if (BitMessage::ShutDownThread.joinable())
			{
				BitMessage::ShutDownThread.join();
			}
		});

		BitMessage::BMClient = new BitMRC(BITMESSAGE_OBJECT_STORAGE_FILENAME, BITMESSAGE_KEYS_FILENAME);
		BitMessage::BMClient->init();
		BitMessage::BMClient->defaultTTL = 1 * 60 * 60; // 1 hour

		if (BitMessage::BMClient->PrivAddresses.empty())
		{
			if (!this->InitAddr())
			{
				// Generate a random address ready to use
				throw std::runtime_error("Failed to prepare source address for exception handling");
			}

			BitMessage::BMClient->save();
		}

		BitMessage::BMClient->start();

#ifdef DEBUG
		Command::Add("bm_send", [] (Command::Params params)
		{
			if (params.length() < 3) return;

			ustring pubAddrString;
			pubAddrString.fromString(params[1]);

			PubAddr pubAddr;
			if (pubAddr.loadAddr(pubAddrString))
			{
				ustring msg;
				msg.fromString(params.join(2));

				Logger::Print("Sending message (this may take a while)...\n");
				BitMessage::BMClient->sendMessage(msg, pubAddr, BitMessage::BMClient->PrivAddresses[0]);
				Logger::Print("Message sent.\n");
			}
			else
			{
				Logger::Print("Address not correct!\n");
			}
		});

		Command::Add("bm_sendb", [] (Command::Params params)
		{
			if (params.length() < 2) return;

			ustring msg;
			msg.appendVarString(params.join(1));
			Logger::Print("Sending broadcast...\n");
			BitMessage::BMClient->sendBroadcast(msg, BitMessage::BMClient->PrivAddresses[0]);
			Logger::Print("Broadcast done.\n");
		});

		Command::Add("bm_check_messages", [] (Command::Params)
		{
			if (!BitMessage::BMClient) return;

			while (BitMessage::BMClient->new_messages.size() > 0)
			{
				auto msg = BitMessage::BMClient->new_messages.pop();
				Logger::Print("New message:\nFrom: %s\nTo: %s\nMessage:\n%s\n", msg.from.data(), msg.to.data(), msg.info.data());
			}
		});

		Command::Add("bm_check_connections", [] (Command::Params)
		{
			if (!BitMessage::BMClient) return;
			std::shared_lock<std::shared_timed_mutex> mlock(BitMessage::BMClient->mutex_nodes);

			for (auto& node : BitMessage::BMClient->Nodes)
			{
				switch (node->state) 
				{
				case 0: // Not connected
					Logger::Print("%s: Disconnected\n", node->Ip.data());
					break;
				case 1: // Connecting
					Logger::Print("%s: Connecting\n", node->Ip.data());
					break;
				case 2: // Connected
					Logger::Print("%s: Connected\n", node->Ip.data());
					break;
				case 3: // Reconnecting
					Logger::Print("%s: Reconnecting\n", node->Ip.data());
					break;
				}
			}

			mlock.unlock();
		});

		Command::Add("bm_check_privatekey", [] (Command::Params)
		{
			if (!BitMessage::BMClient) return;
			std::shared_lock<std::shared_timed_mutex> mlock(BitMessage::BMClient->mutex_priv);

			if (BitMessage::BMClient->PrivAddresses.empty())
			{
				Logger::Print("No private key\n");
			}
			else
			{
				for (auto& addr : BitMessage::BMClient->PrivAddresses)
				{
					Logger::Print("%s\n", addr.getAddress().data());
				}
			}

			mlock.unlock();
		});

		Command::Add("bm_check_publickey", [] (Command::Params)
		{
			if (!BitMessage::BMClient) return;
			std::shared_lock<std::shared_timed_mutex> mlock(BitMessage::BMClient->mutex_pub);

			if (BitMessage::BMClient->PubAddresses.empty())
			{
				Logger::Print("No public key\n");
			}
			else
				for (auto& addr : BitMessage::BMClient->PubAddresses)
				{
					Logger::Print("%s (waiting for public key: %s)\n", addr.getAddress().data(), addr.waitingPubKey() ? "yes" : "no");
				}

			mlock.unlock();
		});

		Command::Add("bm_save", [] (Command::Params)
		{
			BitMessage::Save();
		});

		Command::Add("bm_address_public", [] (Command::Params params)
		{
			if (!BitMessage::BMClient) return;
			if (params.length() < 2) return;

			ustring addre;
			addre.fromString(params.join(1));

			PubAddr address;
			if (address.loadAddr(addre))
			{
				Logger::Print("Asking public key!\n");
				BitMessage::BMClient->getPubKey(address);
				Logger::Print("Asked! check publickey for news on that address!\n");
			}
			else
			{
				Logger::Print("Address not correct!\n");
			}
		});

		Command::Add("bm_address_broadcast", [] (Command::Params params)
		{
			if (!BitMessage::BMClient) return;
			if (params.length() < 2) return;

			ustring addre;
			addre.fromString(params.join(1));
			PubAddr address;
			if (address.loadAddr(addre))
			{
				Logger::Print("Adding subscription!\n");
				BitMessage::BMClient->addSubscription(address);
			}
			else
			{
				Logger::Print("Address not correct!\n");
			}
		});
#endif
	}

	void BitMessage::ShutDown()
	{
		if (BitMessage::BMClient)
		{
			BitMessage::Save();
			delete BitMessage::BMClient;
			BitMessage::BMClient = nullptr;
		}
	}

	BitMessage::~BitMessage()
	{
		BitMessage::ShutDown();
	}

	void BitMessage::SetDefaultTTL(time_t ttl)
	{
		if (!BitMessage::BMClient) return;
		BitMessage::BMClient->defaultTTL = ttl;
	}

	bool BitMessage::RequestPublicKey(std::string targetAddress)
	{
		if (!BitMessage::BMClient) return false;

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
		BitMessage::BMClient->getPubKey(pubAddr);
		return true;
	}

	PubAddr* BitMessage::FindPublicKey(PubAddr address)
	{
		if (!BitMessage::BMClient) return nullptr;
		std::shared_lock<std::shared_timed_mutex> mlock(BitMessage::BMClient->mutex_pub);

		PubAddr* retval = nullptr;

		for (auto& pubKey : BitMessage::BMClient->PubAddresses)
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
		if (!BitMessage::BMClient) return false;

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
		auto resolvedAddress = BitMessage::FindPublicKey(address);
		if (resolvedAddress != nullptr &&
			!resolvedAddress->waitingPubKey() && !resolvedAddress->getPubEncryptionKey().empty())
			return true;

		if (resolvedAddress == nullptr ||
			(!resolvedAddress->waitingPubKey() && resolvedAddress->getPubEncryptionKey().empty()))
		{
			// Request public key
			BitMessage::BMClient->getPubKey(address);
			resolvedAddress = BitMessage::FindPublicKey(address);
		}

		BitMessage::Save();

		// TODO: Wait for public key by using signaling in BitMRC, needs to be done directly in the fork.
		while (resolvedAddress->waitingPubKey())
		{
			std::this_thread::sleep_for(1500ms);
		}

		BitMessage::Save();

		return true;
	}

	bool BitMessage::Subscribe(std::string targetAddress)
	{
		if (!BitMessage::BMClient) return false;

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
		BitMessage::BMClient->addSubscription(pubAddr);
		return true;
	}

	bool BitMessage::SendMsg(std::string targetAddress, std::string message, time_t ttl)
	{
		if (!BitMessage::BMClient) return false;

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
		{
			BitMessage::BMClient->sendMessage(messageU, pubAddr, BitMessage::BMClient->PrivAddresses[0], ttl);
		}
		else
		{
			BitMessage::BMClient->sendMessage(messageU, pubAddr, BitMessage::BMClient->PrivAddresses[0]);
		}

		return true;
	}

	bool BitMessage::SendBroadcast(std::string message, time_t ttl)
	{
		if (!BitMessage::BMClient) return false;

		// Convert message to ustring
		ustring messageU;
		messageU.fromString(message);

		// TODO - Set mutex on priv when accessing first private address
		if (ttl > 0)
		{
			BitMessage::BMClient->sendBroadcast(messageU, BitMessage::BMClient->PrivAddresses[0], ttl);
		}
		else
		{
			BitMessage::BMClient->sendBroadcast(messageU, BitMessage::BMClient->PrivAddresses[0]);
		}

		return true;
	}

	bool BitMessage::InitAddr()
	{
		if (!BitMessage::BMClient) return false;

#ifdef DEBUG
		Logger::Print("Generating BM address...\n");
#endif
		Addr myAddress;
		if (!myAddress.generateRandom())
		{
			return false;
		}

		BitMessage::BMClient->addAddr(myAddress);
		return true;
	}

	void BitMessage::Save()
	{
		if (BitMessage::BMClient)
		{
			BitMessage::BMClient->save();
		}
	}
}

#endif
