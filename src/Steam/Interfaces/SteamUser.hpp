namespace Steam
{
	class User
	{
	public:
		virtual int GetHSteamUser();
		virtual bool LoggedOn();
		virtual SteamID GetSteamID();

		virtual int InitiateGameConnection(void *pAuthBlob, int cbMaxAuthBlob, SteamID steamIDGameServer, unsigned int unIPServer, unsigned short usPortServer, bool bSecure);
		virtual void TerminateGameConnection(unsigned int unIPServer, unsigned short usPortServer);
		virtual void TrackAppUsageEvent(SteamID gameID, int eAppUsageEvent, const char *pchExtraInfo = "");
		virtual bool GetUserDataFolder(char *pchBuffer, int cubBuffer);
		virtual void StartVoiceRecording();
		virtual void StopVoiceRecording();
		virtual int GetCompressedVoice(void *pDestBuffer, unsigned int cbDestBufferSize, unsigned int *nBytesWritten);
		virtual int DecompressVoice(void *pCompressed, unsigned int cbCompressed, void *pDestBuffer, unsigned int cbDestBufferSize, unsigned int *nBytesWritten);
		virtual unsigned int GetAuthSessionTicket(void *pTicket, int cbMaxTicket, unsigned int *pcbTicket);
		virtual int BeginAuthSession(const void *pAuthTicket, int cbAuthTicket, SteamID steamID);
		virtual void EndAuthSession(SteamID steamID);
		virtual void CancelAuthTicket(unsigned int hAuthTicket);
		virtual unsigned int UserHasLicenseForApp(SteamID steamID, unsigned int appID);

		static ::Utils::Cryptography::ECDSA::Key GuidKey;
	};
}
