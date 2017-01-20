#pragma once

namespace Steam
{
	class RemoteStorage
	{
	protected:
		~RemoteStorage() = default;

	public:
		virtual bool FileWrite(const char *pchFile, const void *pvData, int cubData);
		virtual int GetFileSize(const char *pchFile);
		virtual int FileRead(const char *pchFile, void *pvData, int cubDataToRead);
		virtual bool FileExists(const char *pchFile);
		virtual int GetFileCount();
		virtual const char *GetFileNameAndSize(int iFile, int *pnFileSizeInBytes);
		virtual bool GetQuota(int *pnTotalBytes, int *puAvailableBytes);
	};
}
