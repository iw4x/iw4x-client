#include <STDInclude.hpp>
#include "Components/Modules/StartupMessages.hpp"

namespace Steam
{
	uint64_t Callbacks::CallID = 0;
	std::map<uint64_t, bool> Callbacks::Calls;
	std::map<uint64_t, Callbacks::Base*> Callbacks::ResultHandlers;
	std::vector<Callbacks::Result> Callbacks::Results;
	std::vector<Callbacks::Base*> Callbacks::CallbackList;
	std::recursive_mutex Callbacks::Mutex;

	uint64_t Callbacks::RegisterCall()
	{
		std::lock_guard<std::recursive_mutex> _(Callbacks::Mutex);
		Callbacks::Calls[++Callbacks::CallID] = false;
		return Callbacks::CallID;
	}

	void Callbacks::RegisterCallback(Callbacks::Base* handler, int callback)
	{
		std::lock_guard<std::recursive_mutex> _(Callbacks::Mutex);
		handler->SetICallback(callback);
		Callbacks::CallbackList.push_back(handler);
	}

	void Callbacks::RegisterCallResult(uint64_t call, Callbacks::Base* result)
	{
		std::lock_guard<std::recursive_mutex> _(Callbacks::Mutex);
		Callbacks::ResultHandlers[call] = result;
	}

	void Callbacks::ReturnCall(void* data, int size, int type, uint64_t call)
	{
		std::lock_guard<std::recursive_mutex> _(Callbacks::Mutex);

		Callbacks::Result result;

		Callbacks::Calls[call] = true;

		result.call = call;
		result.data = data;
		result.size = size;
		result.type = type;

		Callbacks::Results.push_back(result);
	}

	void Callbacks::RunCallbacks()
	{
		std::lock_guard<std::recursive_mutex> _(Callbacks::Mutex);

		auto results = Callbacks::Results;
		Callbacks::Results.clear();

		for (auto result : results)
		{
			if (Callbacks::ResultHandlers.contains(result.call))
			{
				Callbacks::ResultHandlers[result.call]->Run(result.data, false, result.call);
			}

			for (auto callback : Callbacks::CallbackList)
			{
				if (callback && callback->GetICallback() == result.type)
				{
					callback->Run(result.data, false, 0);
				}
			}

			if (result.data)
			{
				::Utils::Memory::Free(result.data);
			}
		}
	}

	void Callbacks::RunCallback(int32_t callback, void* data)
	{
		std::lock_guard<std::recursive_mutex> _(Callbacks::Mutex);

		for (auto cb : Callbacks::CallbackList)
		{
			if (cb && cb->GetICallback() == callback)
			{
				cb->Run(data);
			}
		}
	}

	void Callbacks::Uninitialize()
	{
		std::lock_guard<std::recursive_mutex> _(Callbacks::Mutex);

		for (auto result : Callbacks::Results)
		{
			if (result.data)
			{
				::Utils::Memory::Free(result.data);
			}
		}

		Callbacks::Results.clear();
	}

	bool Enabled()
	{
		static std::optional<bool> flag;

		if (!flag.has_value())
		{
			flag = Components::Flags::HasFlag("nosteam");
		}

		return !flag.value();
	}

	extern "C"
	{
		bool SteamAPI_Init()
		{
			Proxy::SetGame(10190);

			if (!Proxy::Inititalize())
			{
#ifdef _DEBUG
				OutputDebugStringA("Steam proxy not initialized properly");
#endif
				Components::StartupMessages::AddMessage("Warning:\nUnable to connect to Steam. Steam features will be unavailable");
			}
			else
			{
				Proxy::SetMod("IW4x: Modern Warfare 2");
				Proxy::RunGame();
			}

			return true;
		}

		void SteamAPI_RegisterCallResult(Callbacks::Base* result, uint64_t call)
		{
			Callbacks::RegisterCallResult(call, result);
		}

		void SteamAPI_RegisterCallback(Callbacks::Base* handler, int callback)
		{
			Callbacks::RegisterCallback(handler, callback);
		}

		void SteamAPI_RunCallbacks()
		{
			Callbacks::RunCallbacks();
			Proxy::RunFrame();
		}

		void SteamAPI_Shutdown()
		{
			Proxy::Uninititalize();
			Callbacks::Uninitialize();
		}

		void SteamAPI_UnregisterCallResult()
		{
		}

		void SteamAPI_UnregisterCallback()
		{
		}


		bool SteamGameServer_Init()
		{
			return true;
		}

		void SteamGameServer_RunCallbacks()
		{
		}

		void SteamGameServer_Shutdown()
		{
		}


		Steam::Friends* SteamFriends()
		{
			static Steam::Friends iFriends;
			return &iFriends;
		}

		Steam::Matchmaking* SteamMatchmaking()
		{
			static Steam::Matchmaking iMatchmaking;
			return &iMatchmaking;
		}

		Steam::GameServer* SteamGameServer()
		{
			static Steam::GameServer iGameServer;
			return &iGameServer;
		}

		Steam::MasterServerUpdater* SteamMasterServerUpdater()
		{
			static Steam::MasterServerUpdater iMasterServerUpdater;
			return &iMasterServerUpdater;
		}

		Steam::Networking* SteamNetworking()
		{
			static Steam::Networking iNetworking;
			return &iNetworking;
		}

		Steam::RemoteStorage* SteamRemoteStorage()
		{
			static Steam::RemoteStorage iRemoteStorage;
			return &iRemoteStorage;
		}

		Steam::User* SteamUser()
		{
			static Steam::User iUser;
			return &iUser;
		}

		Steam::Utils* SteamUtils()
		{
			static Steam::Utils iUtils;
			return &iUtils;
		}
	}
}
