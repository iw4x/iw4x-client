#pragma once

namespace Steam
{
  // As with the other interfaces, x64 calls a newer version of this one
  // than x86 does. We lay it out the way Steamworks SDK 1.20 does
  // STEAMREMOTESTORAGE_INTERFACE_VERSION008, the earliest version whose
  // slots match the ones that x64 calls (see Friends for why the
  // earliest).
  //
  class RemoteStorage
  {
  public:
    virtual bool FileWrite(const char *pchFile, const void *pvData, int cubData);
    virtual int FileRead(const char *pchFile, void *pvData, int cubDataToRead);
    virtual bool FileForget(const char *pchFile);
    virtual bool FileDelete(const char *pchFile);
    virtual uint64_t FileShare(const char *pchFile);
    virtual bool SetSyncPlatforms(const char *pchFile, int eRemoteStoragePlatform);
    virtual uint64_t FileWriteStreamOpen(const char *pchFile);
    virtual bool FileWriteStreamWriteChunk(uint64_t writeHandle, const void *pvData, int cubData);
    virtual bool FileWriteStreamClose(uint64_t writeHandle);
    virtual bool FileWriteStreamCancel(uint64_t writeHandle);
    virtual bool FileExists(const char *pchFile);
    virtual bool FilePersisted(const char *pchFile);
    virtual int GetFileSize(const char *pchFile);
    virtual int64_t GetFileTimestamp(const char *pchFile);
    virtual int GetSyncPlatforms(const char *pchFile);
    virtual int GetFileCount();
    virtual const char *GetFileNameAndSize(int iFile, int *pnFileSizeInBytes);
    virtual bool GetQuota(int *pnTotalBytes, int *puAvailableBytes);
    virtual bool IsCloudEnabledForAccount();
    virtual bool IsCloudEnabledForApp();
    virtual void SetCloudEnabledForApp(bool bEnabled);
    virtual uint64_t UGCDownload(uint64_t hContent);
    virtual bool GetUGCDownloadProgress(uint64_t hContent, int *pnBytesDownloaded, int *pnBytesExpected);
    virtual bool GetUGCDetails(uint64_t hContent, unsigned int *pnAppID, char **ppchName, int *pnFileSizeInBytes, SteamID *pSteamIDOwner);
    virtual int UGCRead(uint64_t hContent, void *pvData, int cubDataToRead);
    virtual int GetCachedUGCCount();
    virtual uint64_t GetCachedUGCHandle(int iCachedContent);
    virtual void GetFileListFromServer();
    virtual bool FileFetch(const char *pchFile);
    virtual bool FilePersist(const char *pchFile);
    virtual bool SynchronizeToClient();
    virtual bool SynchronizeToServer();
    virtual bool ResetFileRequestState();
    virtual uint64_t PublishWorkshopFile(const char *pchFile, const char *pchPreviewFile, unsigned int nConsumerAppId, const char *pchTitle, const char *pchDescription, int eVisibility, void *pTags, int eWorkshopFileType);
    virtual uint64_t CreatePublishedFileUpdateRequest(uint64_t unPublishedFileId);
    virtual bool UpdatePublishedFileFile(uint64_t updateHandle, const char *pchFile);
    virtual bool UpdatePublishedFilePreviewFile(uint64_t updateHandle, const char *pchPreviewFile);
    virtual bool UpdatePublishedFileTitle(uint64_t updateHandle, const char *pchTitle);
    virtual bool UpdatePublishedFileDescription(uint64_t updateHandle, const char *pchDescription);
    virtual bool UpdatePublishedFileVisibility(uint64_t updateHandle, int eVisibility);
    virtual bool UpdatePublishedFileTags(uint64_t updateHandle, void *pTags);
    virtual uint64_t CommitPublishedFileUpdate(uint64_t updateHandle);
    virtual uint64_t GetPublishedFileDetails(uint64_t unPublishedFileId);
    virtual uint64_t DeletePublishedFile(uint64_t unPublishedFileId);
    virtual uint64_t EnumerateUserPublishedFiles(unsigned int unStartIndex);
    virtual uint64_t SubscribePublishedFile(uint64_t unPublishedFileId);
    virtual uint64_t EnumerateUserSubscribedFiles(unsigned int unStartIndex);
    virtual uint64_t UnsubscribePublishedFile(uint64_t unPublishedFileId);
    virtual bool UpdatePublishedFileSetChangeDescription(uint64_t updateHandle, const char *pchChangeDescription);
    virtual uint64_t GetPublishedItemVoteDetails(uint64_t unPublishedFileId);
    virtual uint64_t UpdateUserPublishedItemVote(uint64_t unPublishedFileId, bool bVoteUp);
    virtual uint64_t GetUserPublishedItemVoteDetails(uint64_t unPublishedFileId);
    virtual uint64_t EnumerateUserSharedWorkshopFiles(SteamID steamId, unsigned int unStartIndex, void *pRequiredTags, void *pExcludedTags);
    virtual uint64_t PublishVideo(int eVideoProvider, const char *pchVideoAccount, const char *pchVideoIdentifier, const char *pchPreviewFile, unsigned int nConsumerAppId, const char *pchTitle, const char *pchDescription, int eVisibility, void *pTags);
    virtual uint64_t SetUserPublishedFileAction(uint64_t unPublishedFileId, int eAction);
    virtual uint64_t EnumeratePublishedFilesByUserAction(int eAction, unsigned int unStartIndex);
    virtual uint64_t EnumeratePublishedWorkshopFiles(int eEnumerationType, unsigned int unStartIndex, unsigned int unCount, unsigned int unDays, void *pTags, void *pUserTags);
  };
}
