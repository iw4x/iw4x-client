#pragma once

namespace Steam
{
	class Utils
	{
	public:
		virtual unsigned int GetSecondsSinceAppActive();
		virtual unsigned int GetSecondsSinceComputerActive();
		virtual int GetConnectedUniverse();
		virtual unsigned int GetServerRealTime();
		virtual const char *GetIPCountry();
		virtual bool GetImageSize(int iImage, unsigned int *pnWidth, unsigned int *pnHeight);
		virtual bool GetImageRGBA(int iImage, unsigned char *pubDest, int nDestBufferSize);
		virtual bool GetCSERIPPort(unsigned int *unIP, unsigned short *usPort);
		virtual unsigned char GetCurrentBatteryPower();
		virtual unsigned int GetAppID();
		virtual bool IsAPICallCompleted(unsigned __int64 hSteamAPICall, bool *pbFailed);
		virtual int GetAPICallFailureReason(unsigned __int64 hSteamAPICall);
		virtual bool GetAPICallResult(unsigned __int64 hSteamAPICall, void *pCallback, int cubCallback, int iCallbackExpected, bool *pbFailed);
		virtual void RunFrame();
		virtual unsigned int GetIPCCallCount();
		virtual void SetWarningMessageHook(void(*pFunction)(int hpipe, const char *message));
		virtual bool IsOverlayEnabled();
		virtual bool BOverlayNeedsPresent();
	};
}
