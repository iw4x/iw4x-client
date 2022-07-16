#pragma once

namespace Components
{
	class Events : public Component
	{
	public:
		typedef void(ClientCallback)(int clientNum);
		typedef void(Callback)();

		Events();

		// Server side
		static void OnClientDisconnect(const Utils::Slot<ClientCallback>& callback);

		// Client side
		static void OnSteamDisconnect(const Utils::Slot<Callback>& callback);

		static void OnVMShutdown(const Utils::Slot<Callback>& callback);

		static void OnClientInit(const Utils::Slot<Callback>& callback);

		// Client & Server (triggered once)
		static void OnSVInit(const Utils::Slot<Callback>& callback);

	private:
		static Utils::Signal<ClientCallback> ClientDisconnectSignal;
		static Utils::Signal<Callback> SteamDisconnectSignal;
		static Utils::Signal<Callback> ShutdownSystemSignal;
		static Utils::Signal<Callback> ClientInitSignal;
		static Utils::Signal<Callback> ServerInitSignal;

		static void ClientDisconnect_Hk(int clientNum);
		static void SteamDisconnect_Hk();
		static void Scr_ShutdownSystem_Hk(unsigned char sys);
		static void CL_InitOnceForAllClients_HK();
		static void SV_Init_Hk();
	};
}
