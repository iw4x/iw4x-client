#include <STDInclude.hpp>

STEAM_IGNORE_WARNINGS_START

namespace Steam
{
	bool Networking::SendP2PPacket(SteamID steamIDRemote, const void *pubData, unsigned int cubData, int eP2PSendType)
	{
		return false;
	}

	bool Networking::IsP2PPacketAvailable(unsigned int *pcubMsgSize)
	{
		return false;
	}

	bool Networking::ReadP2PPacket(void *pubDest, unsigned int cubDest, unsigned int *pcubMsgSize, SteamID *psteamIDRemote)
	{
		return false;
	}

	bool Networking::AcceptP2PSessionWithUser(SteamID steamIDRemote)
	{
		return false;
	}

	bool Networking::CloseP2PSessionWithUser(SteamID steamIDRemote)
	{
		return false;
	}

	bool Networking::GetP2PSessionState(SteamID steamIDRemote, void *pConnectionState)
	{
		return false;
	}

	unsigned int Networking::CreateListenSocket(int nVirtualP2PPort, unsigned int nIP, unsigned short nPort, bool bAllowUseOfPacketRelay)
	{
		return NULL;
	}

	unsigned int Networking::CreateP2PConnectionSocket(SteamID steamIDTarget, int nVirtualPort, int nTimeoutSec, bool bAllowUseOfPacketRelay)
	{
		return NULL;
	}

	unsigned int Networking::CreateConnectionSocket(unsigned int nIP, unsigned short nPort, int nTimeoutSec)
	{
		return NULL;
	}

	bool Networking::DestroySocket(unsigned int hSocket, bool bNotifyRemoteEnd)
	{
		return false;
	}

	bool Networking::DestroyListenSocket(unsigned int hSocket, bool bNotifyRemoteEnd)
	{
		return false;
	}

	bool Networking::SendDataOnSocket(unsigned int hSocket, void *pubData, unsigned int cubData, bool bReliable)
	{
		return false;
	}

	bool Networking::IsDataAvailableOnSocket(unsigned int hSocket, unsigned int *pcubMsgSize)
	{
		return false;
	}

	bool Networking::RetrieveDataFromSocket(unsigned int hSocket, void *pubDest, unsigned int cubDest, unsigned int *pcubMsgSize)
	{
		return false;
	}

	bool Networking::IsDataAvailable(unsigned int hListenSocket, unsigned int *pcubMsgSize, unsigned int *phSocket)
	{
		return false;
	}

	bool Networking::RetrieveData(unsigned int hListenSocket, void *pubDest, unsigned int cubDest, unsigned int *pcubMsgSize, unsigned int *phSocket)
	{
		return false;
	}

	bool Networking::GetSocketInfo(unsigned int hSocket, SteamID *pSteamIDRemote, int *peSocketStatus, unsigned int *punIPRemote, unsigned short *punPortRemote)
	{
		return false;
	}

	bool Networking::GetListenSocketInfo(unsigned int hListenSocket, unsigned int *pnIP, unsigned short *pnPort)
	{
		return false;
	}

	int Networking::GetSocketConnectionType(unsigned int hSocket)
	{
		return 0;
	}

	int Networking::GetMaxPacketSize(unsigned int hSocket)
	{
		return 0;
	}
}

STEAM_IGNORE_WARNINGS_END
