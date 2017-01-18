#include "STDInclude.hpp"

namespace Steam
{
	::Utils::Library Proxy::Client;
	::Utils::Library Proxy::Overlay;

	ISteamClient008* Proxy::SteamClient;
	IClientEngine*   Proxy::ClientEngine;
	IClientUser*     Proxy::ClientUser;

	void* Proxy::SteamPipe;
	void* Proxy::SteamUser;

	Friends* Proxy::SteamFriends;
	Utils* Proxy::SteamUtils;

	uint32_t Proxy::AppId;

	void Proxy::SetGame(uint32_t appId)
	{
		Proxy::AppId = appId;

		SetEnvironmentVariableA("SteamAppId", ::Utils::String::VA("%lu", appId));
		SetEnvironmentVariableA("SteamGameId", ::Utils::String::VA("%llu", appId & 0xFFFFFF));

		::Utils::IO::WriteFile("steam_appid.txt", ::Utils::String::VA("%lu", appId), false);
	}

	void Proxy::SetMod(std::string mod)
	{
#if 0
		if (!Proxy::ClientUser) return;

		GameID_t gameID;
		gameID.m_nType = 1; // k_EGameIDTypeGameMod
		gameID.m_nAppID = Proxy::AppId & 0xFFFFFF;
		gameID.m_nModID = 0x01010101;

		char ourPath[MAX_PATH] = { 0 };
		GetModuleFileNameA(GetModuleHandle(NULL), ourPath, sizeof(ourPath));

		char ourDirectory[MAX_PATH] = { 0 };
		GetCurrentDirectoryA(sizeof(ourDirectory), ourDirectory);

		char blob[1] = { 0 };
		std::string cmdline = ::Utils::String::VA("\"%s\" -parentProc %d", ourPath, GetCurrentProcessId());
		Proxy::ClientUser->SpawnProcess(blob, 0, ourPath, cmdline.data(), 0, ourDirectory, gameID, Proxy::AppId, mod.data(), 0);
#endif
	}

	void Proxy::RunMod()
	{
		char* command = "-parentProc ";
		char* parentProc = strstr(GetCommandLineA(), command);

		OutputDebugStringA(GetCommandLineA());

		if (parentProc)
		{
			parentProc += strlen(command);
			int pid = atoi(parentProc);

			HANDLE processHandle = OpenProcess(SYNCHRONIZE, FALSE, pid);

			if (processHandle && processHandle != INVALID_HANDLE_VALUE)
			{
				WaitForSingleObject(processHandle, INFINITE);
				CloseHandle(processHandle);
			}

			TerminateProcess(GetCurrentProcess(), 0);
		}
	}

	bool Proxy::Inititalize()
	{
		std::string directoy = Proxy::GetSteamDirectory();
		if (directoy.empty()) return false;

		SetDllDirectoryA(Proxy::GetSteamDirectory().data());

		Proxy::Client = ::Utils::Library(STEAMCLIENT_LIB, false);
		Proxy::Overlay = ::Utils::Library(GAMEOVERLAY_LIB, false);
		if (!Proxy::Client.valid() || !Proxy::Overlay.valid()) return false;

		Proxy::SteamClient = Proxy::Client.get<ISteamClient008*(const char*, int*)>("CreateInterface")("SteamClient008", nullptr);
		if(!Proxy::SteamClient) return false;

		Proxy::SteamPipe = Proxy::SteamClient->CreateSteamPipe();
		if (!Proxy::SteamPipe) return false;

		Proxy::SteamUser = Proxy::SteamClient->ConnectToGlobalUser(Proxy::SteamPipe);
		if (!Proxy::SteamUser) return false;

		Proxy::ClientEngine = Proxy::Client.get<IClientEngine*(const char*, int*)>("CreateInterface")("CLIENTENGINE_INTERFACE_VERSION004", nullptr);
		if (!Proxy::ClientEngine) return false;

		Proxy::ClientUser = Proxy::ClientEngine->GetIClientUser(Proxy::SteamUser, Proxy::SteamPipe, "CLIENTUSER_INTERFACE_VERSION001");
		if (!Proxy::ClientUser) return false;

		Proxy::SteamFriends = reinterpret_cast<Friends*>(Proxy::SteamClient->GetISteamFriends(Proxy::SteamUser, Proxy::SteamPipe, "SteamFriends005"));
		if (!Proxy::SteamFriends) return false;

		Proxy::SteamUtils = reinterpret_cast<Utils*>(Proxy::SteamClient->GetISteamFriends(Proxy::SteamUser, Proxy::SteamPipe, "SteamUtils005"));
		if (!Proxy::SteamUtils) return false;

		return true;
	}

	void Proxy::Uninititalize()
	{
		if (Proxy::SteamClient && Proxy::SteamPipe)
		{
			if (Proxy::SteamUser)
			{
				//Proxy::SteamClient->ReleaseUser(Proxy::SteamPipe, Proxy::SteamUser);
			}

			//Proxy::SteamClient->ReleaseSteamPipe(Proxy::SteamPipe);
		}

		Proxy::Client = ::Utils::Library();
		Proxy::Overlay = ::Utils::Library();
	}

	std::string Proxy::GetSteamDirectory()
	{
		HKEY hRegKey;
		char SteamPath[MAX_PATH];
		if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, STEAM_REGISTRY_PATH, 0, KEY_QUERY_VALUE, &hRegKey) == ERROR_SUCCESS)
		{
			DWORD dwLength = sizeof(SteamPath);
			RegQueryValueExA(hRegKey, "InstallPath", NULL, NULL, reinterpret_cast<BYTE*>(SteamPath), &dwLength);
			RegCloseKey(hRegKey);

			return SteamPath;
		}

		return "";
	}

	void Proxy::SetOverlayNotificationPosition(uint32_t eNotificationPosition)
	{
		if (Proxy::Overlay.valid())
		{
			Proxy::Overlay.get<void(uint32_t)>("SetNotificationPosition")(eNotificationPosition);
		}
	}

	bool Proxy::IsOverlayEnabled()
	{
		if (Proxy::Overlay.valid())
		{
			return Proxy::Overlay.get<bool()>("IsOverlayEnabled")();
		}

		return false;
	}

	bool Proxy::BOverlayNeedsPresent()
	{
		if (Proxy::Overlay.valid())
		{
			return Proxy::Overlay.get<bool()>("BOverlayNeedsPresent")();
		}

		return false;
	}
}
