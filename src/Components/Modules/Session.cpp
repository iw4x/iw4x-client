#include <proto/session.pb.h>

#include "Session.hpp"

namespace Components
{
	volatile bool Session::Terminate;
	std::thread Session::Thread;

	std::recursive_mutex Session::Mutex;
	std::unordered_map<Network::Address, Session::Frame> Session::Sessions;
	std::unordered_map<Network::Address, std::queue<std::shared_ptr<Session::Packet>>> Session::PacketQueue;

	Utils::Cryptography::ECC::Key Session::SignatureKey;

	std::unordered_map<std::string, Network::networkCallback> Session::PacketHandlers;

	std::queue<std::pair<Network::Address, std::string>> Session::SignatureQueue;

	void Session::Send(const Network::Address& target, const std::string& command, const std::string& data)
	{
#ifdef DISABLE_SESSION
		class DelayedResend
		{
		public:
			Network::Address target;
			std::string command;
			std::string data;
		};

		auto* delayData = new DelayedResend;
		delayData->target = target;
		delayData->command = command;
		delayData->data = data;

		Network::SendCommand(target, command, data);

		Scheduler::Once([delayData]
		{
			Network::SendCommand(delayData->target, delayData->command, delayData->data);
			delete delayData;
		}, Scheduler::Pipeline::MAIN, 500ms + std::chrono::milliseconds(std::rand() % 200));
#else
		std::lock_guard _(Session::Mutex);

		auto queue = Session::PacketQueue.find(target);
		if (queue == Session::PacketQueue.end())
		{
			Session::PacketQueue[target] = std::queue<std::shared_ptr<Session::Packet>>();
			queue = Session::PacketQueue.find(target);
			if (queue == Session::PacketQueue.end()) Logger::Error(Game::ERR_FATAL, "Failed to enqueue session packet!\n");
		}

		std::shared_ptr<Session::Packet> packet = std::make_shared<Session::Packet>();
		packet->command = command;
		packet->data = data;
		packet->tries = 0;

		queue->second.push(packet);
#endif
	}

	void Session::Handle(const std::string& packet, const Network::networkCallback& callback)
	{
#ifdef DISABLE_SESSION
		Network::OnClientPacket(packet, callback);
#else
		std::lock_guard _(Session::Mutex);
		Session::PacketHandlers[packet] = callback;
#endif
	}

	void Session::RunFrame()
	{
		std::lock_guard _(Session::Mutex);

		for (auto queue = Session::PacketQueue.begin(); queue != Session::PacketQueue.end();)
		{
			if (queue->second.empty())
			{
				queue = Session::PacketQueue.erase(queue);
				continue;
			}

			std::shared_ptr<Session::Packet> packet = queue->second.front();
			if (!packet->lastTry.has_value() || !packet->tries || (packet->lastTry.has_value() && packet->lastTry->elapsed(SESSION_TIMEOUT)))
			{
				if (packet->tries <= SESSION_MAX_RETRIES)
				{
					packet->tries++;
					if(!packet->lastTry.has_value()) packet->lastTry.emplace(Utils::Time::Point());
					packet->lastTry->update();

					Network::SendCommand(queue->first, "sessionSyn");
				}
				else
				{
					queue->second.pop();
				}
			}

			++queue;
		}
	}

	void Session::HandleSignatures()
	{
		while (!Session::SignatureQueue.empty())
		{
			std::unique_lock<std::recursive_mutex> lock(Session::Mutex);
			auto signature = Session::SignatureQueue.front();
			Session::SignatureQueue.pop();

			auto queue = Session::PacketQueue.find(signature.first);
			if (queue == Session::PacketQueue.end()) continue;

			if (!queue->second.empty())
			{
				std::shared_ptr<Session::Packet> packet = queue->second.front();
				queue->second.pop();
				lock.unlock();

				Proto::Session::Packet dataPacket;
				dataPacket.set_publickey(Session::SignatureKey.getPublicKey());
				dataPacket.set_signature(Utils::Cryptography::ECC::SignMessage(Session::SignatureKey, signature.second));
				dataPacket.set_command(packet->command);
				dataPacket.set_data(packet->data);

				Network::SendCommand(signature.first, "sessionFin", dataPacket.SerializeAsString());
				std::this_thread::sleep_for(3ms);
			}
		}
	}

	Session::Session()
	{
#ifndef DISABLE_SESSION
		Session::SignatureKey = Utils::Cryptography::ECC::GenerateKey(512);
		//Scheduler::OnFrame(Session::RunFrame);

		Session::Terminate = false;
		Session::Thread = std::thread([]()
		{
			Com_InitThreadData();

			while (!Session::Terminate)
			{
				Session::RunFrame();
				Session::HandleSignatures();
				Game::Sys_Sleep(20);
			}
		});

		Network::OnPacket("sessionSyn", [](const Network::Address& address, [[maybe_unused]] const std::string& data)
		{
			Session::Frame frame;
			frame.challenge = Utils::Cryptography::Rand::GenerateChallenge();

			std::lock_guard _(Session::Mutex);
			Session::Sessions[address] = frame;

			Network::SendCommand(address, "sessionAck", frame.challenge);
		});

		Network::OnPacket("sessionAck", [](const Network::Address& address, [[maybe_unused]] const std::string& data)
		{
			std::lock_guard _(Session::Mutex);
			Session::SignatureQueue.push({ address, data });
		});

		Network::OnPacket("sessionFin", [](Network::Address& address, [[maybe_unused]] const std::string& data)
		{
			std::lock_guard _(Session::Mutex);

			auto frame = Session::Sessions.find(address);
			if (frame == Session::Sessions.end()) return;

			std::string challenge = frame->second.challenge;
			Session::Sessions.erase(frame);

			Proto::Session::Packet dataPacket;
			if (!dataPacket.ParseFromString(data)) return;

			Utils::Cryptography::ECC::Key publicKey;
			publicKey.set(dataPacket.publickey());

			if (!Utils::Cryptography::ECC::VerifyMessage(publicKey, challenge, dataPacket.signature())) return;

			auto handler = Session::PacketHandlers.find(dataPacket.command());
			if (handler == Session::PacketHandlers.end()) return;

			handler->second(address, dataPacket.data());
		});
#endif
	}

	Session::~Session()
	{
		std::lock_guard _(Session::Mutex);
		Session::PacketHandlers.clear();
		Session::PacketQueue.clear();
		Session::SignatureQueue = std::queue<std::pair<Network::Address, std::string>>();

		Session::SignatureKey.free();
	}

	void Session::preDestroy()
	{
		Session::Terminate = true;
		if (Session::Thread.joinable())
		{
			Session::Thread.join();
		}
	}
}
