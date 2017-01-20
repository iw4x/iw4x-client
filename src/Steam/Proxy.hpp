#pragma once

#ifdef _WIN64
#define GAMEOVERLAY_LIB "gameoverlayrenderer64.dll"
#define STEAMCLIENT_LIB "steamclient64.dll"
#define STEAM_REGISTRY_PATH "Software\\Wow6432Node\\Valve\\Steam"
#else
#define GAMEOVERLAY_LIB "gameoverlayrenderer.dll"
#define STEAMCLIENT_LIB "steamclient.dll"
#define STEAM_REGISTRY_PATH "Software\\Valve\\Steam"
#endif

namespace Steam
{
	class ISteamClient008
	{
	public:
		virtual void* CreateSteamPipe() = 0;
		virtual bool ReleaseSteamPipe(void* hSteamPipe) = 0;
		virtual void* ConnectToGlobalUser(void* hSteamPipe) = 0;
		virtual void* CreateLocalUser(void* *phSteamPipe, int eAccountType) = 0;
		virtual void ReleaseUser(void* hSteamPipe, void* hUser) = 0;
		virtual void *GetISteamUser(void* hSteamUser, void* hSteamPipe, const char *pchVersion) = 0;
		virtual void *GetISteamGameServer(void* hSteamUser, void* hSteamPipe, const char *pchVersion) = 0;
		virtual void SetLocalIPBinding(uint32_t unIP, uint16_t usPort) = 0;
		virtual void *GetISteamFriends(void* hSteamUser, void* hSteamPipe, const char *pchVersion) = 0;
		virtual void *GetISteamUtils(void* hSteamPipe, const char *pchVersion) = 0;
		virtual void *GetISteamMatchmaking(void* hSteamUser, void* hSteamPipe, const char *pchVersion) = 0;
		virtual void *GetISteamMasterServerUpdater(void* hSteamUser, void* hSteamPipe, const char *pchVersion) = 0;
		virtual void *GetISteamMatchmakingServers(void* hSteamUser, void* hSteamPipe, const char *pchVersion) = 0;
		virtual void *GetISteamGenericInterface(void* hSteamUser, void* hSteamPipe, const char *pchVersion) = 0;
		virtual void *GetISteamUserStats(void* hSteamUser, void* hSteamPipe, const char *pchVersion) = 0;
		virtual void *GetISteamApps(void* hSteamUser, void* hSteamPipe, const char *pchVersion) = 0;
		virtual void *GetISteamNetworking(void* hSteamUser, void* hSteamPipe, const char *pchVersion) = 0;
		virtual void*GetISteamRemoteStorage(void* hSteamuser, void* hSteamPipe, const char *pchVersion) = 0;
		virtual void RunFrame() = 0;
		virtual uint32_t GetIPCCallCount() = 0;
		virtual void SetWarningMessageHook(void* pFunction) = 0;
	};

	class IClientUser
	{
	public:
		virtual int32_t GetHSteamUser() = 0;
		virtual void LogOn(bool bInteractive, SteamID steamID) = 0;
		virtual void LogOnWithPassword(bool bInteractive, const char * pchLogin, const char * pchPassword) = 0;
		virtual void LogOnAndCreateNewSteamAccountIfNeeded(bool bInteractive) = 0;
		virtual int LogOnConnectionless() = 0;
		virtual void LogOff() = 0;
		virtual bool BLoggedOn() = 0;
		virtual int GetLogonState() = 0;
		virtual bool BConnected() = 0;
		virtual bool BTryingToLogin() = 0;
		virtual SteamID GetSteamID() = 0;
		virtual SteamID GetConsoleSteamID() = 0;
		virtual bool IsVACBanned(uint32_t nGameID) = 0;
		virtual bool RequireShowVACBannedMessage(uint32_t nAppID) = 0;
		virtual void AcknowledgeVACBanning(uint32_t nAppID) = 0;
		virtual void SetSteam2Ticket(uint8_t* pubTicket, int32_t cubTicket) = 0;
		virtual bool BExistsSteam2Ticket() = 0;
		virtual bool SetEmail(const char *pchEmail) = 0;
		virtual bool SetConfigString(int eRegistrySubTree, const char *pchKey, const char *pchValue) = 0;
		virtual bool GetConfigString(int eRegistrySubTree, const char *pchKey, char *pchValue, int32_t cbValue) = 0;
		virtual bool SetConfigInt(int eRegistrySubTree, const char *pchKey, int32_t iValue) = 0;
		virtual bool GetConfigInt(int eRegistrySubTree, const char *pchKey, int32_t *pValue) = 0;
		virtual bool GetConfigStoreKeyName(int eRegistrySubTree, const char *pchKey, char *pchStoreName, int32_t cbStoreName) = 0;
		virtual int32_t InitiateGameConnection(void *pOutputBlob, int32_t cbBlobMax, SteamID steamIDGS, uint64_t gameID, uint32_t unIPServer, uint16_t usPortServer, bool bSecure) = 0;
		virtual int32_t InitiateGameConnectionOld(void *pOutputBlob, int32_t cbBlobMax, SteamID steamIDGS, uint64_t gameID, uint32_t unIPServer, uint16_t usPortServer, bool bSecure, void *pvSteam2GetEncryptionKey, int32_t cbSteam2GetEncryptionKey) = 0;
		virtual void TerminateGameConnection(uint32_t unIPServer, uint16_t usPortServer) = 0;
		virtual bool TerminateAppMultiStep(uint32_t, uint32_t) = 0;
		virtual void SetSelfAsPrimaryChatDestination() = 0;
		virtual bool IsPrimaryChatDestination() = 0;
		virtual void RequestLegacyCDKey(uint32_t iAppID) = 0;
		virtual bool AckGuestPass(const char *pchGuestPassCode) = 0;
		virtual bool RedeemGuestPass(const char *pchGuestPassCode) = 0;
		virtual uint32_t GetGuestPassToGiveCount() = 0;
		virtual uint32_t GetGuestPassToRedeemCount() = 0;
		virtual bool GetGuestPassToGiveInfo(uint32_t nPassIndex, uint64_t *pgidGuestPassID, uint32_t* pnPackageID, uint32_t* pRTime32Created, uint32_t* pRTime32Expiration, uint32_t* pRTime32Sent, uint32_t* pRTime32Redeemed, char* pchRecipientAddress, int32_t cRecipientAddressSize) = 0;
		virtual bool GetGuestPassToRedeemInfo(uint32_t nPassIndex, uint64_t *pgidGuestPassID, uint32_t* pnPackageID, uint32_t* pRTime32Created, uint32_t* pRTime32Expiration, uint32_t* pRTime32Sent, uint32_t* pRTime32Redeemed) = 0;
		virtual bool GetGuestPassToRedeemSenderName(uint32_t nPassIndex, char* pchSenderName, int32_t cSenderNameSize) = 0;
		virtual void AcknowledgeMessageByGID(const char *pchMessageGID) = 0;
		virtual bool SetLanguage(const char *pchLanguage) = 0;
		virtual void TrackAppUsageEvent(uint64_t gameID, int32_t eAppUsageEvent, const char *pchExtraInfo = "") = 0;
		virtual int32_t RaiseConnectionPriority(int eConnectionPriority) = 0;
		virtual void ResetConnectionPriority(int32_t hRaiseConnectionPriorityPrev) = 0;
		virtual void SetAccountNameFromSteam2(const char *pchAccountName) = 0;
		virtual bool SetPasswordFromSteam2(const char *pchPassword) = 0;
		virtual bool BHasCachedCredentials(const char * pchUnk) = 0;
		virtual bool SetAccountNameForCachedCredentialLogin(const char *pchAccountName, bool bUnk) = 0;
		virtual void SetLoginInformation(const char *pchAccountName, const char *pchPassword, bool bRememberPassword) = 0;
		virtual void ClearAllLoginInformation() = 0;
		virtual void SetAccountCreationTime(uint32_t rtime32Time) = 0;
		virtual uint64_t RequestWebAuthToken() = 0;
		virtual bool GetCurrentWebAuthToken(char *pchBuffer, int32_t cubBuffer) = 0;
		virtual bool GetLanguage(char* pchLanguage, int32_t cbLanguage) = 0;
		virtual bool BIsCyberCafe() = 0;
		virtual bool BIsAcademicAccount() = 0;
		virtual void CreateAccount(const char *pchAccountName, const char *pchNewPassword, const char *pchNewEmail, int32_t iQuestion, const char *pchNewQuestion, const char *pchNewAnswer) = 0;
		virtual uint64_t ResetPassword(const char *pchAccountName, const char *pchOldPassword, const char *pchNewPassword, const char *pchValidationCode, const char *pchAnswer) = 0;
		virtual void TrackNatTraversalStat(const void *pNatStat) = 0;
		virtual void TrackSteamUsageEvent(int eSteamUsageEvent, const uint8_t *pubKV, uint32_t cubKV) = 0;
		virtual void TrackSteamGUIUsage(const char *) = 0;
		virtual void SetComputerInUse() = 0;
		virtual bool BIsGameRunning(uint64_t gameID) = 0;
		virtual uint64_t GetCurrentSessionToken() = 0;
		virtual bool BUpdateAppOwnershipTicket(uint32_t nAppID, bool bOnlyUpdateIfStale, bool bIsDepot) = 0;
		virtual bool RequestCustomBinary(const char *pszAbsolutePath, uint32_t nAppID, bool bForceUpdate, bool bAppLaunchRequest) = 0;
		virtual int GetCustomBinariesState(uint32_t unAppID, uint32_t *punProgress) = 0;
		virtual int RequestCustomBinaries(uint32_t unAppID, bool, bool, uint32_t *) = 0;
		virtual void SetCellID(uint32_t cellID) = 0;
		virtual void SetWinningPingTimeForCellID(uint32_t uPing) = 0;
		virtual const char *GetUserBaseFolder() = 0;
		virtual bool GetUserDataFolder(uint64_t gameID, char* pchBuffer, int32_t cubBuffer) = 0;
		virtual bool GetUserConfigFolder(char *pchBuffer, int32_t cubBuffer) = 0;
		virtual bool GetAccountName(char* pchAccountName, uint32_t cb) = 0;
		virtual bool GetAccountName(SteamID userID, char * pchAccountName, uint32_t cb) = 0;
		virtual bool IsPasswordRemembered() = 0;
		virtual bool RequiresLegacyCDKey(uint32_t nAppID, bool * pbUnk) = 0;
		virtual bool GetLegacyCDKey(uint32_t nAppID, char* pchKeyData, int32_t cbKeyData) = 0;
		virtual bool SetLegacyCDKey(uint32_t nAppID, const char* pchKeyData) = 0;
		virtual bool WriteLegacyCDKey(uint32_t nAppID) = 0;
		virtual void RemoveLegacyCDKey(uint32_t nAppID) = 0;
		virtual void RequestLegacyCDKeyFromApp(uint32_t nMainAppID, uint32_t nDLCAppID) = 0;
		virtual bool BIsAnyGameRunning() = 0;
		virtual void TestAvailablePassword(const uint8_t *pubDigestPassword, int32_t cubDigestPassword) = 0;
		virtual void GetSteamGuardDetails() = 0;
		virtual void ChangePassword(const char *pchOldPassword, const char *pchNewPassword) = 0;
		virtual void ChangeEmail(const char *, const char *pchEmail) = 0;
		virtual void ChangeSecretQuestionAndAnswer(const char *, int32_t iQuestion, const char *pchNewQuestion, const char *pchNewAnswer) = 0;
		virtual void SetSteam2FullASTicket(uint8_t* pubTicket, int32_t cubTicket) = 0;
		virtual int32_t GetSteam2FullASTicket(uint8_t* pubTicket, int32_t cubTicket) = 0;
		virtual bool GetEmail(char* pchEmail, int32_t cchEmail, bool* pbValidated) = 0;
		virtual void RequestForgottenPasswordEmail(const char *pchAccountName, const char *pchTriedPassword) = 0;
		virtual void FindAccountsByEmailAddress(const char *pchEmailAddress) = 0;
		virtual void FindAccountsByCdKey(const char *pchCdKey) = 0;
		virtual void GetNumAccountsWithEmailAddress(const char * pchEmailAddress) = 0;
		virtual void IsAccountNameInUse(const char * pchAccountName) = 0;
		virtual void Test_FakeConnectionTimeout() = 0;
		virtual bool RunInstallScript(uint32_t *pAppIDs, int32_t cAppIDs, const char *pchInstallPath, const char *pchLanguage, bool bUninstall) = 0;
		virtual uint32_t IsInstallScriptRunning() = 0;
		virtual bool GetInstallScriptState(char* pchDescription, uint32_t cchDescription, uint32_t* punNumSteps, uint32_t* punCurrStep) = 0;

		virtual void Test1() = 0;
		virtual void Test2() = 0;

		virtual bool SpawnProcess(void *lpVACBlob, uint32_t cbBlobSize, const char *lpApplicationName, const char *lpCommandLine, uint32_t dwCreationFlags, const char *lpCurrentDirectory, GameID_t gameID, uint32_t nAppID, const char *pchGameName, uint32_t uUnk) = 0;
		virtual uint32_t GetAppOwnershipTicketLength(uint32_t nAppID) = 0;
		virtual uint32_t GetAppOwnershipTicketData(uint32_t nAppID, void *pvBuffer, uint32_t cbBufferLength) = 0;
		virtual uint32_t GetAppOwnershipTicketExtendedData(uint32_t nAppID, void *pvBuffer, uint32_t cbBufferLength, uint32_t* piAppId, uint32_t* piSteamId, uint32_t* piSignature, uint32_t* pcbSignature) = 0;
		virtual int32_t GetMarketingMessageCount() = 0;
		virtual bool GetMarketingMessage(int32_t cMarketingMessage, uint64_t* gidMarketingMessageID, char* pubMsgUrl, int32_t cubMessageUrl, int *eMarketingMssageFlags) = 0;
		virtual uint32_t GetAuthSessionTicket(void *pMyAuthTicket, int32_t cbMaxMyAuthTicket, uint32_t* pcbAuthTicket) = 0;
		virtual int BeginAuthSession(void const* pTheirAuthTicket, int32_t cbTicket, SteamID steamID) = 0;
		virtual void EndAuthSession(SteamID steamID) = 0;
		virtual void CancelAuthTicket(uint32_t hAuthTicket) = 0;
		virtual int IsUserSubscribedAppInTicket(SteamID steamID, uint32_t appID) = 0;
		virtual void AdvertiseGame(uint64_t gameID, SteamID steamIDGameServer, uint32_t unIPServer, uint16_t usPortServer) = 0;
		virtual uint64_t RequestEncryptedAppTicket(const void *pUserData, int32_t cbUserData) = 0;
		virtual bool GetEncryptedAppTicket(void *pTicket, int32_t cbMaxTicket, uint32_t *pcbTicket) = 0;
		virtual int32_t GetGameBadgeLevel(int32_t nSeries, bool bFoil) = 0;
		virtual int32_t GetPlayerSteamLevel() = 0;
		virtual void SetAccountLimited(bool bAccountLimited) = 0;
		virtual bool BIsAccountLimited() = 0;
		virtual void SetAccountCommunityBanned(bool bBanned) = 0;
		virtual bool BIsAccountCommunityBanned() = 0;
		virtual void SetLimitedAccountCanInviteFriends(bool bCanInviteFriends) = 0;
		virtual bool BLimitedAccountCanInviteFriends() = 0;
		virtual void SendValidationEmail() = 0;
		virtual bool BGameConnectTokensAvailable() = 0;
		virtual int32_t NumGamesRunning() = 0;
		virtual uint64_t GetRunningGameID(int32_t iGame) = 0;
		virtual uint32_t GetAccountSecurityPolicyFlags() = 0;
		virtual void RequestChangeEmail(const char *pchPassword, int32_t eRequestType) = 0;
		virtual void ChangePasswordWithCode(const char *pchOldPassword, const char *pchCode, const char *pchNewPassword) = 0;
		virtual void ChangeEmailWithCode(const char *pchPassword, const char *pchCode, const char *pchEmail, bool bFinal) = 0;
		virtual void ChangeSecretQuestionAndAnswerWithCode(const char *pchPassword, const char *pchCode, const char *pchNewQuestion, const char *pchNewAnswer) = 0;
		virtual void SetClientStat(int eStat, int64_t llValue, uint32_t nAppID, uint32_t nDepotID, uint32_t nCellID) = 0;
		virtual void VerifyPassword(const char *pchPassword) = 0;
		virtual bool BSupportUser() = 0;
		virtual bool BNeedsSSANextSteamLogon() = 0;
		virtual void ClearNeedsSSANextSteamLogon() = 0;
		virtual bool BIsAppOverlayEnabled(uint64_t gameID) = 0;
		virtual bool BIsBehindNAT() = 0;
		virtual uint32_t GetMicroTxnAppID(uint64_t gidTransID) = 0;
		virtual uint64_t GetMicroTxnOrderID(uint64_t gidTransID) = 0;
		virtual bool BGetMicroTxnPrice(uint64_t gidTransID, uint64_t *pamtTotal, uint64_t *pamtTax, bool *pbVat, uint64_t * pUnk) = 0;
		virtual int32_t GetMicroTxnLineItemCount(uint64_t gidTransID) = 0;
		virtual bool BGetMicroTxnLineItem(uint64_t gidTransID, uint32_t unLineItem, uint64_t *pamt, uint32_t *punQuantity, char *pchDescription, uint32_t cubDescriptionLength, int32_t *pRecurringTimeUnit, uint8_t *pRecurringFrequency, uint64_t *pRecurringAmount, bool * pbUnk) = 0;
		virtual bool BIsSandboxMicroTxn(uint64_t gidTransID, bool* pbSandbox) = 0;
		virtual bool BMicroTxnRequiresCachedPmtMethod(uint64_t gidTransID, bool *pbRequired) = 0;
		virtual uint64_t AuthorizeMicroTxn(uint64_t gidTransID, int eMicroTxnAuthResponse) = 0;
		virtual bool BGetWalletBalance(bool *pbHasWallet, uint64_t *pamtBalance) = 0;
		virtual uint64_t RequestMicroTxnInfo(uint64_t gidTransID) = 0;
		virtual bool BGetAppMinutesPlayed(uint32_t nAppId, int32_t *pnForever, int32_t *pnLastTwoWeeks) = 0;
		virtual uint32_t GetAppLastPlayedTime(uint32_t nAppId) = 0;
		virtual bool BGetGuideURL(uint32_t uAppID, char *pchURL, uint32_t cchURL) = 0;
		virtual void PostUIResultToClientJob(uint64_t ulJobID, int eResult) = 0;
		virtual bool BPromptToVerifyEmail() = 0;
		virtual bool BPromptToChangePassword() = 0;
		virtual bool BAccountLocked() = 0;
		virtual bool BAccountShouldShowLockUI() = 0;
		virtual bool BAccountLockedByIPT() = 0;
		virtual int32_t GetCountAuthedComputers() = 0;
		virtual int GetSteamGuardProvider() = 0;
		virtual bool GetSteamGuardRequireCodeByDefault() = 0;
		virtual bool ShowSteamGuardProviderOptions() = 0;
		virtual bool SteamGuardProviderMobileIsOption() = 0;
		virtual bool BSteamGuardNewMachineNotification() = 0;
		virtual uint32_t GetSteamGuardEnabledTime() = 0;
		virtual bool GetSteamGuardHistoryEntry(int32_t iEntryIndex, uint32_t *puTimestamp, uint32_t *puIP, bool *pbIsRemembered, char *pchGeolocInfo, int32_t cchGeolocInfo, char * pchUnk, int32_t cbUnk) = 0;
		virtual void SetSteamGuardNewMachineDialogResponse(bool bIsApproved, bool bIsWizardComplete) = 0;
		virtual void RequestSteamGuardCodeForOtherLogin() = 0;
		virtual bool BAccountCanUseIPT() = 0;
		virtual void ChangeTwoFactorAuthOptions(int32_t eOption) = 0;
		virtual void ChangeSteamGuardOptions(const char * pchUnk, int eProvider, bool bRequireCode) = 0;
		virtual void Set2ndFactorAuthCode(const char* pchAuthCode, bool bDontRememberComputer) = 0;
		virtual void SetUserMachineName(const char * pchMachineName) = 0;
		virtual bool GetUserMachineName(char * pchMachineName, int32_t cbMachineName) = 0;
		virtual bool BAccountHasIPTConfig() = 0;
		virtual bool GetEmailDomainFromLogonFailure(char * pchEmailDomain, int32_t cbEmailDomain) = 0;
		virtual bool BIsSubscribedApp(uint32_t nAppId) = 0;
		virtual uint64_t RegisterActivationCode(const char * pchActivationCode) = 0;
		virtual void OptionalDLCInstallation(uint32_t nAppID, uint32_t uUnk, bool bInstall) = 0;
		virtual void AckSystemIM(uint64_t) = 0;
		virtual uint64_t RequestSpecialSurvey(uint32_t uSurveyId) = 0;
		virtual uint64_t SendSpecialSurveyResponse(uint32_t uSurveyId, const uint8_t * pubData, uint32_t cubData) = 0;
		virtual void RequestNotifications() = 0;
		virtual bool GetAppOwnershipInfo(uint32_t unAppId, uint32_t* pRTime32Created, char* pchCountry) = 0;
		virtual void SendGameWebCallback(uint32_t unAppId, const char *szData) = 0;
		virtual bool BIsCurrentlyStreaming() = 0;
		virtual void RequestStopStreaming() = 0;
		virtual void OnBigPictureStreamingResult(bool, void *) = 0;
		virtual void OnBigPictureStreamingDone() = 0;
		virtual void OnBigPictureStreamRestarting() = 0;
		virtual void LockParentalLock() = 0;
		virtual bool UnlockParentalLock(const char * pchUnk) = 0;
		virtual bool BIsParentalLockEnabled() = 0;
		virtual bool BIsParentalLockLocked() = 0;
		virtual void BlockApp(uint32_t unAppID) = 0;
		virtual void UnblockApp(uint32_t unAppID) = 0;
		virtual bool BIsAppBlocked(uint32_t unAppID) = 0;
		virtual bool BIsAppInBlockList(uint32_t unAppID) = 0;
		virtual void BlockFeature(int eParentalFeature) = 0;
		virtual void UnblockFeature(int eParentalFeature) = 0;
		virtual bool BIsFeatureBlocked(int eParentalFeature) = 0;
		virtual bool BIsFeatureInBlockList(int eParentalFeature) = 0;
		virtual uint32_t GetParentalUnlockTime() = 0;
		virtual bool BGetSerializedParentalSettings(void * pBuffer) = 0;
		virtual bool BSetParentalSettings(void * pBuffer) = 0;
		virtual bool BDisableParentalSettings() = 0;
		virtual bool BGetParentalWebToken(void *, void *) = 0;
		virtual bool BCanLogonOfflineMode() = 0;
		virtual int LogOnOfflineMode() = 0;
		virtual int ValidateOfflineLogonTicket(const char * pchUnk) = 0;
		virtual bool BGetOfflineLogonTicket(const char * pchUnk, void * pTicket) = 0;
		virtual void UploadLocalClientLogs() = 0;
		virtual void SetAsyncNotificationEnabled(uint32_t, bool) = 0;
		virtual bool BIsOtherSessionPlaying(uint32_t *) = 0;
		virtual bool BKickOtherPlayingSession() = 0;
		virtual uint32_t GetStreamPortForGame(uint64_t gameID) = 0;
	};

	class IClientEngine
	{
	public:
		virtual void* CreateSteamPipe() = 0;
		virtual bool BReleaseSteamPipe(void* hSteamPipe) = 0;
		virtual void* CreateGlobalUser(void** phSteamPipe) = 0;
		virtual void* ConnectToGlobalUser(void* hSteamPipe) = 0;
		virtual void* CreateLocalUser(void** phSteamPipe, int eAccountType) = 0;
		virtual void CreatePipeToLocalUser(void* hSteamUser, void** phSteamPipe) = 0;
		virtual void ReleaseUser(void* hSteamPipe, void* hUser) = 0;
		virtual bool IsValidHSteamUserPipe(void* hSteamPipe, void* hUser) = 0;
		virtual IClientUser *GetIClientUser(void* hSteamUser, void* hSteamPipe, char const* pchVersion) = 0;
		virtual void *GetIClientGameServer(void* hSteamUser, void* hSteamPipe, const char *pchVersion) = 0;
		virtual void SetLocalIPBinding(uint32_t unIP, uint16_t usPort) = 0;
		virtual char const *GetUniverseName(int eUniverse) = 0;
		virtual void *GetIClientFriends(void* hSteamUser, void* hSteamPipe, char const* pchVersion) = 0;
		virtual void *GetIClientUtils(void* hSteamPipe, char const* pchVersion) = 0;
		virtual void *GetIClientBilling(void* hSteamUser, void* hSteamPipe, char const* pchVersion) = 0;
		virtual void *GetIClientMatchmaking(void* hSteamUser, void* hSteamPipe, char const* pchVersion) = 0;
		virtual void *GetIClientApps(void* hSteamUser, void* hSteamPipe, char const* pchVersion) = 0;
		virtual void *GetIClientMatchmakingServers(void* hSteamUser, void* hSteamPipe, char const* pchVersion) = 0;
		virtual void RunFrame() = 0;
		virtual uint32_t GetIPCCallCount() = 0;
		virtual void *GetIClientUserStats(void* hSteamUser, void* hSteamPipe, char const* pchVersion) = 0;
		virtual void *GetIClientGameServerStats(void* hSteamUser, void* hSteamPipe, char const* pchVersion) = 0;
		virtual void *GetIClientNetworking(void* hSteamUser, void* hSteamPipe, char const* pchVersion) = 0;
		virtual void *GetIClientRemoteStorage(void* hSteamUser, void* hSteamPipe, char const* pchVersion) = 0;
		virtual void *GetIClientScreenshots(void* hSteamUser, void* hSteamPipe, char const* pchVersion) = 0;
		virtual void SetWarningMessageHook(void* pFunction) = 0;
		virtual void *GetIClientGameCoordinator(void* hSteamUser, void* hSteamPipe, const char *pchVersion) = 0;
		virtual void SetOverlayNotificationPosition(int eNotificationPosition) = 0;
		virtual void SetOverlayNotificationInsert(int32_t, int32_t) = 0;
		virtual bool HookScreenshots(bool bHook) = 0;
		virtual bool IsOverlayEnabled() = 0;
		virtual bool GetAPICallResult(void* hSteamPipe, uint64_t hSteamAPICall, void* pCallback, int cubCallback, int iCallbackExpected, bool* pbFailed) = 0;
		virtual void *GetIClientProductBuilder(void* hSteamUser, void* hSteamPipe, char const* pchVersion) = 0;
		virtual void *GetIClientDepotBuilder(void* hSteamUser, void* hSteamPipe, char const* pchVersion) = 0;
		virtual void *GetIClientNetworkDeviceManager(void* hSteamPipe, char const* pchVersion) = 0;
		virtual void ConCommandInit(void *pAccessor) = 0;
		virtual void *GetIClientAppManager(void* hSteamUser, void* hSteamPipe, char const* pchVersion) = 0;
		virtual void *GetIClientConfigStore(void* hSteamUser, void* hSteamPipe, char const* pchVersion) = 0;
		virtual bool BOverlayNeedsPresent() = 0;
		virtual void *GetIClientGameStats(void* hSteamUser, void* hSteamPipe, char const* pchVersion) = 0;
		virtual void *GetIClientHTTP(void* hSteamUser, void* hSteamPipe, char const* pchVersion) = 0;
		virtual bool BShutdownIfAllPipesClosed() = 0;
		virtual void *GetIClientAudio(void* hSteamUser, void* hSteamPipe, char const* pchVersion) = 0;
		virtual void *GetIClientMusic(void* hSteamUser, void* hSteamPipe, char const* pchVersion) = 0;
		virtual void *GetIClientUnifiedMessages(void* hSteamUser, void* hSteamPipe, char const* pchVersion) = 0;
		virtual void *GetIClientController(void* hSteamUser, void* hSteamPipe, char const* pchVersion) = 0;
		virtual void *GetIClientParentalSettings(void* hSteamUser, void* hSteamPipe, char const* pchVersion) = 0;
		virtual void *GetIClientStreamLauncher(void* hSteamUser, void* hSteamPipe, char const* pchVersion) = 0;
		virtual void *GetIClientDeviceAuth(void* hSteamUser, void* hSteamPipe, char const* pchVersion) = 0;
		virtual void *GetIClientRemoteClientManager(void* hSteamPipe, char const* pchVersion) = 0;
		virtual void *GetIClientStreamClient(void* hSteamUser, void* hSteamPipe, char const* pchVersion) = 0;
		virtual void *GetIClientShortcuts(void* hSteamUser, void* hSteamPipe, char const* pchVersion) = 0;
		virtual void *GetIClientRemoteControlManager(void* hSteamPipe, char const* pchVersion) = 0;
		virtual void Set_ClientAPI_CPostAPIResultInProcess(void(*)(uint64_t ulUnk, void * pUnk, uint32_t uUnk, int32_t iUnk)) = 0;
		virtual void Remove_ClientAPI_CPostAPIResultInProcess(void(*)(uint64_t ulUnk, void * pUnk, uint32_t uUnk, int32_t iUnk)) = 0;
		virtual void *GetIClientUGC(void* hSteamUser, void* hSteamPipe, char const* pchVersion) = 0;
		virtual void *GetIClientVR(char const * pchVersion) = 0;
	};

	class Proxy
	{
	public:
		static bool Inititalize();
		static void Uninititalize();

		static void SetGame(uint32_t appId);
		static void SetMod(std::string mod);
		static void RunMod();

		//Overlay related proxies
		static void SetOverlayNotificationPosition(uint32_t eNotificationPosition);
		static bool IsOverlayEnabled();
		static bool BOverlayNeedsPresent();

		static Friends* SteamFriends;
		static Utils* SteamUtils;

	private:
		static ::Utils::Library Client;
		static ::Utils::Library Overlay;

		static ISteamClient008* SteamClient;
		static IClientEngine*   ClientEngine;
		static IClientUser*     ClientUser;

		static void* SteamPipe;
		static void* SteamUser;

		static uint32_t AppId;

		static std::string GetSteamDirectory();
	};
}
