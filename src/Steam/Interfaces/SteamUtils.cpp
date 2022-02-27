#include <STDInclude.hpp>

STEAM_IGNORE_WARNINGS_START

namespace Steam
{
	unsigned int Utils::GetSecondsSinceAppActive()
	{
		return Game::Sys_Milliseconds() / 1000;
	}

	unsigned int Utils::GetSecondsSinceComputerActive()
	{
		return timeGetTime();
	}

	int Utils::GetConnectedUniverse()
	{
		return 1;
	}

	unsigned int Utils::GetServerRealTime()
	{
		static std::optional<unsigned int> timeDelta;
		if(!timeDelta.has_value())
		{
			unsigned int steamTime = static_cast<unsigned int>(time(nullptr));
			if(Steam::Proxy::SteamUtils)
			{
				steamTime = Steam::Proxy::SteamUtils->GetServerRealTime();
			}

			timeDelta.emplace(steamTime - (Game::Sys_Milliseconds() / 1000));
		}

		return timeDelta.value() + (Game::Sys_Milliseconds() / 1000);
	}

	const char* Utils::GetIPCountry()
	{
		if (Steam::Proxy::SteamUtils)
		{
			return Steam::Proxy::SteamUtils->GetIPCountry();
		}

		return "US";
	}

	bool Utils::GetImageSize(int iImage, unsigned int *pnWidth, unsigned int *pnHeight)
	{
		return false;
	}

	bool Utils::GetImageRGBA(int iImage, unsigned char *pubDest, int nDestBufferSize)
	{
		return false;
	}

	bool Utils::GetCSERIPPort(unsigned int *unIP, unsigned short *usPort)
	{
		return false;
	}

	unsigned char Utils::GetCurrentBatteryPower()
	{
		if (Steam::Proxy::SteamUtils)
		{
			return Steam::Proxy::SteamUtils->GetCurrentBatteryPower();
		}

		return 255;
	}

	unsigned int Utils::GetAppID()
	{
		return 10190;
	}

	void Utils::SetOverlayNotificationPosition(int eNotificationPosition)
	{
		Proxy::SetOverlayNotificationPosition(eNotificationPosition);
	}

	bool Utils::IsAPICallCompleted(unsigned __int64 hSteamAPICall, bool *pbFailed)
	{
		return false;
	}

	int Utils::GetAPICallFailureReason(unsigned __int64 hSteamAPICall)
	{
		return -1;
	}

	bool Utils::GetAPICallResult(unsigned __int64 hSteamAPICall, void *pCallback, int cubCallback, int iCallbackExpected, bool *pbFailed)
	{
		return false;
	}

	void Utils::RunFrame()
	{
	}

	unsigned int Utils::GetIPCCallCount()
	{
		return 0;
	}

	void Utils::SetWarningMessageHook(void(*pFunction)(int hpipe, const char *message))
	{
	}

	bool Utils::IsOverlayEnabled()
	{
		return false;
	}

	bool Utils::BOverlayNeedsPresent()
	{
		return false;
	}
}

STEAM_IGNORE_WARNINGS_END
