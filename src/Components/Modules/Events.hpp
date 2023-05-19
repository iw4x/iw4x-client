#pragma once

namespace Components
{
	class Events : public Component
	{
	public:
		using Callback = std::vector<std::function<void()>>;
		using ClientConnectCallback = std::vector<std::function<void(Game::client_s* cl)>>;
		using ClientCallback = std::vector<std::function<void(int clientNum)>>;
		using ClientCmdCallback = std::vector<std::function<void(Game::usercmd_s* cmd)>>;

		Events();

		// Server side
		static void OnClientDisconnect(const std::function<void(int clientNum)>& callback);

		// Server side
		static void OnClientConnect(const std::function<void(Game::client_s* cl)>& callback);

		// Client side
		static void OnSteamDisconnect(const std::function<void()>& callback);

		static void OnVMShutdown(const std::function<void()>& callback);

		static void OnClientInit(const std::function<void()>& callback);

		static void OnClientCmdButtons(const std::function<void(Game::usercmd_s*)>& callback);

		static void OnClientKeyMove(const std::function<void(Game::usercmd_s*)>& callback);

		// Client & Server (triggered once)
		static void OnSVInit(const std::function<void()>& callback);

		// Client & Server (triggered once)
		static void OnDvarInit(const std::function<void()>& callback);

		// Client & Server (triggered once)
		static void OnNetworkInit(const std::function<void()>& callback);

	private:
		static Utils::Concurrency::Container<ClientCallback> ClientDisconnectTasks_;
		static Utils::Concurrency::Container<ClientConnectCallback> ClientConnectTasks_;
		static Utils::Concurrency::Container<Callback> SteamDisconnectTasks_;
		static Utils::Concurrency::Container<Callback> ShutdownSystemTasks_;
		static Utils::Concurrency::Container<Callback> ClientInitTasks_;
		static Utils::Concurrency::Container<Callback> ServerInitTasks_;
		static Utils::Concurrency::Container<Callback> DvarInitTasks_;
		static Utils::Concurrency::Container<Callback> NetworkInitTasks_;

		// For speed this one does not use concurrency container. Be careful
		static ClientCmdCallback ClientCmdButtonsTasks_;
		static ClientCmdCallback ClientKeyMoveTasks_;

		static void ClientDisconnect_Hk(int clientNum);
		static void SV_UserinfoChanged_Hk(Game::client_s* cl);
		static void SteamDisconnect_Hk();
		static void Scr_ShutdownSystem_Hk(unsigned char sys);
		static void CL_InitOnceForAllClients_HK();

		static void CL_CmdButtons(Game::usercmd_s* cmd);
		static void CL_CmdButtons_Stub();
		static void CL_KeyMove(Game::usercmd_s* cmd);
		static void CL_KeyMove_Stub();

		static void SV_Init_Hk();
		static void Com_InitDvars_Hk();

		static void NetworkStart();
		static void NET_OpenSocks_Hk();
	};
}
