#pragma once

namespace Steam
{
  // Note that the game calls the methods of the interface by their slots,
  // and x64 is built against a newer version of it than x86. So we lay it
  // out as Steamworks SDK 1.20 does SteamNetworking005, the earliest
  // version whose slots match the ones x64 calls.
  //
  class Networking
  {
  public:
    virtual bool SendP2PPacket(SteamID steamIDRemote, const void *pubData, unsigned int cubData, int eP2PSendType, int nChannel = 0);
    virtual bool IsP2PPacketAvailable(unsigned int *pcubMsgSize, int nChannel = 0);
    virtual bool ReadP2PPacket(void *pubDest, unsigned int cubDest, unsigned int *pcubMsgSize, SteamID *psteamIDRemote, int nChannel = 0);
    virtual bool AcceptP2PSessionWithUser(SteamID steamIDRemote);
    virtual bool CloseP2PSessionWithUser(SteamID steamIDRemote);
    virtual bool CloseP2PChannelWithUser(SteamID steamIDRemote, int nChannel);
    virtual bool GetP2PSessionState(SteamID steamIDRemote, void *pConnectionState);
    virtual bool AllowP2PPacketRelay(bool bAllow);
    virtual unsigned int CreateListenSocket(int nVirtualP2PPort, unsigned int nIP, unsigned short nPort, bool bAllowUseOfPacketRelay);
    virtual unsigned int CreateP2PConnectionSocket(SteamID steamIDTarget, int nVirtualPort, int nTimeoutSec, bool bAllowUseOfPacketRelay);
    virtual unsigned int CreateConnectionSocket(unsigned int nIP, unsigned short nPort, int nTimeoutSec);
    virtual bool DestroySocket(unsigned int hSocket, bool bNotifyRemoteEnd);
    virtual bool DestroyListenSocket(unsigned int hSocket, bool bNotifyRemoteEnd);
    virtual bool SendDataOnSocket(unsigned int hSocket, void *pubData, unsigned int cubData, bool bReliable);
    virtual bool IsDataAvailableOnSocket(unsigned int hSocket, unsigned int *pcubMsgSize);
    virtual bool RetrieveDataFromSocket(unsigned int hSocket, void *pubDest, unsigned int cubDest, unsigned int *pcubMsgSize);
    virtual bool IsDataAvailable(unsigned int hListenSocket, unsigned int *pcubMsgSize, unsigned int *phSocket);
    virtual bool RetrieveData(unsigned int hListenSocket, void *pubDest, unsigned int cubDest, unsigned int *pcubMsgSize, unsigned int *phSocket);
    virtual bool GetSocketInfo(unsigned int hSocket, SteamID *pSteamIDRemote, int *peSocketStatus, unsigned int *punIPRemote, unsigned short *punPortRemote);
    virtual bool GetListenSocketInfo(unsigned int hListenSocket, unsigned int *pnIP, unsigned short *pnPort);
    virtual int GetSocketConnectionType(unsigned int hSocket);
    virtual int GetMaxPacketSize(unsigned int hSocket);
  };
}
