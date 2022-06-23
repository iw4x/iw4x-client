#include <STDInclude.hpp>

namespace Components
{
	Utils::Signal<Events::ClientCallback> Events::ClientDisconnectSignal;
	Utils::Signal<Events::Callback> Events::SteamDisconnectSignal;
	Utils::Signal<Events::Callback> Events::ShutdownSystemSignal;

	void Events::OnClientDisconnect(const Utils::Slot<ClientCallback>& callback)
	{
		ClientDisconnectSignal.connect(callback);
	}

	void Events::OnSteamDisconnect(const Utils::Slot<Callback>& callback)
	{
		SteamDisconnectSignal.connect(callback);
	}

	void Events::OnVMShutdown(const Utils::Slot<Callback>& callback)
	{
		ShutdownSystemSignal.connect(callback);
	}

	/*
	 * Should be called when a client drops from the server
	 * but not "between levels" (Quake-III-Arena)
	 */
	void Events::ClientDisconnect_Hk(const int clientNum)
	{
		ClientDisconnectSignal(clientNum);

		Utils::Hook::Call<void(int)>(0x4AA430)(clientNum); // ClientDisconnect
	}

	void Events::SteamDisconnect_Hk()
	{
		SteamDisconnectSignal();

		Utils::Hook::Call<void()>(0x467CC0)(); // LiveSteam_Client_SteamDisconnect
	}

	void Events::Scr_ShutdownSystem_Hk(unsigned char sys)
	{
		ShutdownSystemSignal();

		Utils::Hook::Call<void(unsigned char)>(0x421EE0)(sys); // Scr_ShutdownSystem
	}

	Events::Events()
	{
		Utils::Hook(0x625235, ClientDisconnect_Hk, HOOK_CALL).install()->quick(); // SV_FreeClient

		Utils::Hook(0x403582, SteamDisconnect_Hk, HOOK_CALL).install()->quick(); // CL_Disconnect

		Utils::Hook(0x47548B, Scr_ShutdownSystem_Hk, HOOK_CALL).install()->quick(); // G_LoadGame
		Utils::Hook(0x4D06BA, Scr_ShutdownSystem_Hk, HOOK_CALL).install()->quick(); // G_ShutdownGame
	}
}
