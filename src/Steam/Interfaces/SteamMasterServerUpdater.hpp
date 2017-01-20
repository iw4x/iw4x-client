#pragma once

namespace Steam
{
	class MasterServerUpdater
	{
	protected:
		~MasterServerUpdater() = default;

	public:
		virtual void SetActive(bool bActive);
		virtual void SetHeartbeatInterval(int iHeartbeatInterval);
		virtual bool HandleIncomingPacket(const void *pData, int cbData, unsigned int srcIP, unsigned short srcPort);
		virtual int GetNextOutgoingPacket(void *pOut, int cbMaxOut, unsigned int *pNetAdr, unsigned short *pPort);
		virtual void SetBasicServerData(unsigned short nProtocolVersion, bool bDedicatedServer, const char *pRegionName, const char *pProductName, unsigned short nMaxReportedClients, bool bPasswordProtected, const char *pGameDescription);
		virtual void ClearAllKeyValues();
		virtual void SetKeyValue(const char *pKey, const char *pValue);
		virtual void NotifyShutdown();
		virtual bool WasRestartRequested();
		virtual void ForceHeartbeat();
		virtual bool AddMasterServer(const char *pServerAddress);
		virtual bool RemoveMasterServer(const char *pServerAddress);
		virtual int GetNumMasterServers();
		virtual int GetMasterServerAddress(int iServer, char *pOut, int outBufferSize);
	};
}
