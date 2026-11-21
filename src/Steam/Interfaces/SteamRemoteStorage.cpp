#include "SteamRemoteStorage.hpp"

namespace Steam
{
  bool RemoteStorage::FileWrite(const char* /*pchFile*/, const void* /*pvData*/, int /*cubData*/)
  {
    return true;
  }

  int RemoteStorage::FileRead([[maybe_unused]] const char* pchFile, void* /*pvData*/, int /*cubDataToRead*/)
  {
#ifdef _DEBUG
    OutputDebugStringA(pchFile);
#endif
    return 0;
  }

  bool RemoteStorage::FileForget(const char* /*pchFile*/)
  {
    return false;
  }

  bool RemoteStorage::FileDelete(const char* /*pchFile*/)
  {
    return false;
  }

  uint64_t RemoteStorage::FileShare(const char* /*pchFile*/)
  {
    return 0;
  }

  bool RemoteStorage::SetSyncPlatforms(const char* /*pchFile*/, int /*eRemoteStoragePlatform*/)
  {
    return false;
  }

  uint64_t RemoteStorage::FileWriteStreamOpen(const char* /*pchFile*/)
  {
    return 0;
  }

  bool RemoteStorage::FileWriteStreamWriteChunk(uint64_t /*writeHandle*/, const void* /*pvData*/, int /*cubData*/)
  {
    return false;
  }

  bool RemoteStorage::FileWriteStreamClose(uint64_t /*writeHandle*/)
  {
    return false;
  }

  bool RemoteStorage::FileWriteStreamCancel(uint64_t /*writeHandle*/)
  {
    return false;
  }

  bool RemoteStorage::FileExists(const char* /*pchFile*/)
  {
    return false;
  }

  bool RemoteStorage::FilePersisted(const char* /*pchFile*/)
  {
    return false;
  }

  int RemoteStorage::GetFileSize(const char* /*pchFile*/)
  {
    return 0;
  }

  int64_t RemoteStorage::GetFileTimestamp(const char* /*pchFile*/)
  {
    return 0;
  }

  int RemoteStorage::GetSyncPlatforms(const char* /*pchFile*/)
  {
    return 0;
  }

  int RemoteStorage::GetFileCount()
  {
    return 0;
  }

  const char *RemoteStorage::GetFileNameAndSize(int /*iFile*/, int *pnFileSizeInBytes)
  {
    *pnFileSizeInBytes = 0;
    return "";
  }

  bool RemoteStorage::GetQuota(int *pnTotalBytes, int *puAvailableBytes)
  {
    *pnTotalBytes = 0x10000000;
    *puAvailableBytes = 0x10000000;
    return false;
  }

  bool RemoteStorage::IsCloudEnabledForAccount()
  {
    return false;
  }

  bool RemoteStorage::IsCloudEnabledForApp()
  {
    return false;
  }

  void RemoteStorage::SetCloudEnabledForApp(bool /*bEnabled*/)
  {
  }

  uint64_t RemoteStorage::UGCDownload(uint64_t /*hContent*/)
  {
    return 0;
  }

  bool RemoteStorage::GetUGCDownloadProgress(uint64_t /*hContent*/, int* /*pnBytesDownloaded*/, int* /*pnBytesExpected*/)
  {
    return false;
  }

  bool RemoteStorage::GetUGCDetails(uint64_t /*hContent*/, unsigned int* /*pnAppID*/, char** /*ppchName*/, int* /*pnFileSizeInBytes*/, SteamID* /*pSteamIDOwner*/)
  {
    return false;
  }

  int RemoteStorage::UGCRead(uint64_t /*hContent*/, void* /*pvData*/, int /*cubDataToRead*/)
  {
    return 0;
  }

  int RemoteStorage::GetCachedUGCCount()
  {
    return 0;
  }

  uint64_t RemoteStorage::GetCachedUGCHandle(int /*iCachedContent*/)
  {
    return 0;
  }

  void RemoteStorage::GetFileListFromServer()
  {
  }

  bool RemoteStorage::FileFetch(const char* /*pchFile*/)
  {
    return false;
  }

  bool RemoteStorage::FilePersist(const char* /*pchFile*/)
  {
    return false;
  }

  bool RemoteStorage::SynchronizeToClient()
  {
    return false;
  }

  bool RemoteStorage::SynchronizeToServer()
  {
    return false;
  }

  bool RemoteStorage::ResetFileRequestState()
  {
    return false;
  }

  uint64_t RemoteStorage::PublishWorkshopFile(const char* /*pchFile*/, const char* /*pchPreviewFile*/, unsigned int /*nConsumerAppId*/, const char* /*pchTitle*/, const char* /*pchDescription*/, int /*eVisibility*/, void* /*pTags*/, int /*eWorkshopFileType*/)
  {
    return 0;
  }

  uint64_t RemoteStorage::CreatePublishedFileUpdateRequest(uint64_t /*unPublishedFileId*/)
  {
    return 0;
  }

  bool RemoteStorage::UpdatePublishedFileFile(uint64_t /*updateHandle*/, const char* /*pchFile*/)
  {
    return false;
  }

  bool RemoteStorage::UpdatePublishedFilePreviewFile(uint64_t /*updateHandle*/, const char* /*pchPreviewFile*/)
  {
    return false;
  }

  bool RemoteStorage::UpdatePublishedFileTitle(uint64_t /*updateHandle*/, const char* /*pchTitle*/)
  {
    return false;
  }

  bool RemoteStorage::UpdatePublishedFileDescription(uint64_t /*updateHandle*/, const char* /*pchDescription*/)
  {
    return false;
  }

  bool RemoteStorage::UpdatePublishedFileVisibility(uint64_t /*updateHandle*/, int /*eVisibility*/)
  {
    return false;
  }

  bool RemoteStorage::UpdatePublishedFileTags(uint64_t /*updateHandle*/, void* /*pTags*/)
  {
    return false;
  }

  uint64_t RemoteStorage::CommitPublishedFileUpdate(uint64_t /*updateHandle*/)
  {
    return 0;
  }

  uint64_t RemoteStorage::GetPublishedFileDetails(uint64_t /*unPublishedFileId*/)
  {
    return 0;
  }

  uint64_t RemoteStorage::DeletePublishedFile(uint64_t /*unPublishedFileId*/)
  {
    return 0;
  }

  uint64_t RemoteStorage::EnumerateUserPublishedFiles(unsigned int /*unStartIndex*/)
  {
    return 0;
  }

  uint64_t RemoteStorage::SubscribePublishedFile(uint64_t /*unPublishedFileId*/)
  {
    return 0;
  }

  uint64_t RemoteStorage::EnumerateUserSubscribedFiles(unsigned int /*unStartIndex*/)
  {
    return 0;
  }

  uint64_t RemoteStorage::UnsubscribePublishedFile(uint64_t /*unPublishedFileId*/)
  {
    return 0;
  }

  bool RemoteStorage::UpdatePublishedFileSetChangeDescription(uint64_t /*updateHandle*/, const char* /*pchChangeDescription*/)
  {
    return false;
  }

  uint64_t RemoteStorage::GetPublishedItemVoteDetails(uint64_t /*unPublishedFileId*/)
  {
    return 0;
  }

  uint64_t RemoteStorage::UpdateUserPublishedItemVote(uint64_t /*unPublishedFileId*/, bool /*bVoteUp*/)
  {
    return 0;
  }

  uint64_t RemoteStorage::GetUserPublishedItemVoteDetails(uint64_t /*unPublishedFileId*/)
  {
    return 0;
  }

  uint64_t RemoteStorage::EnumerateUserSharedWorkshopFiles(SteamID /*steamId*/, unsigned int /*unStartIndex*/, void* /*pRequiredTags*/, void* /*pExcludedTags*/)
  {
    return 0;
  }

  uint64_t RemoteStorage::PublishVideo(int /*eVideoProvider*/, const char* /*pchVideoAccount*/, const char* /*pchVideoIdentifier*/, const char* /*pchPreviewFile*/, unsigned int /*nConsumerAppId*/, const char* /*pchTitle*/, const char* /*pchDescription*/, int /*eVisibility*/, void* /*pTags*/)
  {
    return 0;
  }

  uint64_t RemoteStorage::SetUserPublishedFileAction(uint64_t /*unPublishedFileId*/, int /*eAction*/)
  {
    return 0;
  }

  uint64_t RemoteStorage::EnumeratePublishedFilesByUserAction(int /*eAction*/, unsigned int /*unStartIndex*/)
  {
    return 0;
  }

  uint64_t RemoteStorage::EnumeratePublishedWorkshopFiles(int /*eEnumerationType*/, unsigned int /*unStartIndex*/, unsigned int /*unCount*/, unsigned int /*unDays*/, void* /*pTags*/, void* /*pUserTags*/)
  {
    return 0;
  }
}
