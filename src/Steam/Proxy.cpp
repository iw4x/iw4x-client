#include "STDInclude.hpp"

namespace Steam
{
	::Utils::Library Proxy::Client;
	::Utils::Library Proxy::Overlay;

	ISteamClient008* Proxy::SteamClient;

	void* Proxy::SteamPipe;
	void* Proxy::SteamUser;

	Friends* Proxy::SteamFriends;
	Utils* Proxy::SteamUtils;

	bool Proxy::Inititalize()
	{
		std::string directoy = Proxy::GetSteamDirectory();
		if (directoy.empty()) return false;

		SetDllDirectoryA(Proxy::GetSteamDirectory().data());

		Proxy::Client = ::Utils::Library(STEAMCLIENT_LIB, false);
		Proxy::Overlay = ::Utils::Library(GAMEOVERLAY_LIB, false);
		if (!Proxy::Client.valid() || !Proxy::Overlay.valid()) return false;

		Proxy::SteamClient = Proxy::Client.get<ISteamClient008*(const char*, int*)>("CreateInterface")("SteamClient008", nullptr);
		Proxy::SteamPipe = Proxy::SteamClient->CreateSteamPipe();
		if (!Proxy::SteamPipe) return false;

		Proxy::SteamUser = Proxy::SteamClient->ConnectToGlobalUser(Proxy::SteamPipe);
		if (!Proxy::SteamUser)
		{
			//Proxy::SteamClient->ReleaseSteamPipe(Proxy::SteamPipe);
			return false;
		}

		Proxy::SteamFriends = reinterpret_cast<Friends*>(Proxy::SteamClient->GetISteamFriends(Proxy::SteamUser, Proxy::SteamPipe, "SteamFriends005"));
		if (!Proxy::SteamFriends)
		{
			//Proxy::SteamClient->ReleaseUser(Proxy::SteamPipe, Proxy::SteamUser);
			//Proxy::SteamClient->ReleaseSteamPipe(Proxy::SteamPipe);
			return false;
		}

		Proxy::SteamUtils = reinterpret_cast<Utils*>(Proxy::SteamClient->GetISteamFriends(Proxy::SteamUser, Proxy::SteamPipe, "SteamUtils005"));
		if (!Proxy::SteamUtils)
		{
			//Proxy::SteamClient->ReleaseUser(Proxy::SteamPipe, Proxy::SteamUser);
			//Proxy::SteamClient->ReleaseSteamPipe(Proxy::SteamPipe);
			return false;
		}

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
