#include <STDInclude.hpp>

STEAM_IGNORE_WARNINGS_START

namespace Steam
{
	void MasterServerUpdater::SetActive(bool bActive)
	{
	}

	void MasterServerUpdater::SetHeartbeatInterval(int iHeartbeatInterval)
	{
	}

	bool MasterServerUpdater::HandleIncomingPacket(const void *pData, int cbData, unsigned int srcIP, unsigned short srcPort)
	{
		return true;
	}

	int MasterServerUpdater::GetNextOutgoingPacket(void *pOut, int cbMaxOut, unsigned int *pNetAdr, unsigned short *pPort)
	{
		return 0;
	}

	void MasterServerUpdater::SetBasicServerData(unsigned short nProtocolVersion, bool bDedicatedServer, const char *pRegionName, const char *pProductName, unsigned short nMaxReportedClients, bool bPasswordProtected, const char *pGameDescription)
	{
	}

	void MasterServerUpdater::ClearAllKeyValues()
	{
	}

	void MasterServerUpdater::SetKeyValue(const char *pKey, const char *pValue)
	{
	}

	void MasterServerUpdater::NotifyShutdown()
	{
	}

	bool MasterServerUpdater::WasRestartRequested()
	{
		return false;
	}

	void MasterServerUpdater::ForceHeartbeat()
	{
	}

	bool MasterServerUpdater::AddMasterServer(const char *pServerAddress)
	{
		return true;
	}

	bool MasterServerUpdater::RemoveMasterServer(const char *pServerAddress)
	{
		return true;
	}

	int MasterServerUpdater::GetNumMasterServers()
	{
		return 0;
	}

	int MasterServerUpdater::GetMasterServerAddress(int iServer, char *pOut, int outBufferSize)
	{
		return 0;
	}
}

STEAM_IGNORE_WARNINGS_END
