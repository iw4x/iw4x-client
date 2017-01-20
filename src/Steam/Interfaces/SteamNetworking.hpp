#pragma once

namespace Steam
{
	class Networking
	{
	public:
		virtual bool SendP2PPacket(SteamID steamIDRemote, const void *pubData, unsigned int cubData, int eP2PSendType);
		virtual bool IsP2PPacketAvailable(unsigned int *pcubMsgSize);
		virtual bool ReadP2PPacket(void *pubDest, unsigned int cubDest, unsigned int *pcubMsgSize, SteamID *psteamIDRemote);
		virtual bool AcceptP2PSessionWithUser(SteamID steamIDRemote);
		virtual bool CloseP2PSessionWithUser(SteamID steamIDRemote);
		virtual bool GetP2PSessionState(SteamID steamIDRemote, void *pConnectionState);
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
