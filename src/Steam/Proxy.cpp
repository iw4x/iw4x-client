#include "STDInclude.hpp"

namespace Steam
{
	HMODULE Proxy::Client = nullptr;
	HMODULE Proxy::Overlay = nullptr;

	bool Proxy::Inititalize()
	{
		std::string directoy = Proxy::GetSteamDirectory();
		if (directoy.empty()) return false;

		SetDllDirectoryA(Proxy::GetSteamDirectory().data());

		Proxy::Client = LoadLibraryA(STEAMCLIENT_LIB);
		Proxy::Overlay = LoadLibraryA(GAMEOVERLAY_LIB);

		return (Proxy::Client && Proxy::Overlay);
	}

	void Proxy::Uninititalize()
	{
		// Freeing libraries causes crashes
		//if (Proxy::Client)  FreeLibrary(Proxy::Client);
		Proxy::Client = nullptr;

		//if (Proxy::Overlay) FreeLibrary(Proxy::Overlay);
		Proxy::Overlay = nullptr;
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
		if (Proxy::Overlay)
		{
			FARPROC SetNotificationPositionFn = GetProcAddress(Proxy::Overlay, "SetNotificationPosition");

			if (SetNotificationPositionFn)
			{
				::Utils::Hook::Call<void(uint32_t)>(SetNotificationPositionFn)(eNotificationPosition);
			}
		}
	}

	bool Proxy::IsOverlayEnabled()
	{
		if (Proxy::Overlay)
		{
			FARPROC IsOverlayEnabledFn = GetProcAddress(Proxy::Overlay, "IsOverlayEnabled");

			if (IsOverlayEnabledFn)
			{
				return ::Utils::Hook::Call<bool()>(IsOverlayEnabledFn)();
			}
		}

		return false;
	}

	bool Proxy::BOverlayNeedsPresent()
	{
		if (Proxy::Overlay)
		{
			FARPROC BOverlayNeedsPresentFn = GetProcAddress(Proxy::Overlay, "BOverlayNeedsPresent");

			if (BOverlayNeedsPresentFn)
			{
				return ::Utils::Hook::Call<bool()>(BOverlayNeedsPresentFn)();
			}
		}

		return false;
	}
}
