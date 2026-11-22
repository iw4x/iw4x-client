#pragma once

namespace Steam
{
  // As with the other interfaces, x64 calls a newer version of this one
  // than x86 does. We lay it out the way Steamworks SDK 1.20 does
  // SteamUtils005, the earliest version whose slots match the ones that
  // x64 calls (see Friends for why the earliest).
  //
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
    virtual void SetOverlayNotificationPosition(int eNotificationPosition);
    virtual bool IsAPICallCompleted(unsigned __int64 hSteamAPICall, bool *pbFailed);
    virtual int GetAPICallFailureReason(unsigned __int64 hSteamAPICall);
    virtual bool GetAPICallResult(unsigned __int64 hSteamAPICall, void *pCallback, int cubCallback, int iCallbackExpected, bool *pbFailed);
    virtual void RunFrame();
    virtual unsigned int GetIPCCallCount();
    virtual void SetWarningMessageHook(void(*pFunction)(int hpipe, const char *message));
    virtual bool IsOverlayEnabled();
    virtual bool BOverlayNeedsPresent();
    virtual uint64_t CheckFileSignature(const char *szFileName);
    virtual void PostPS3SysutilCallback(uint64_t status, uint64_t param, void *userdata);
    virtual bool BIsReadyToShutdown();
    virtual bool BIsPSNOnline();
    virtual void SetPSNGameBootInviteStrings(const char *pchSubject, const char *pchBody);
  };
}
