#include <STDInclude.hpp>

STEAM_IGNORE_WARNINGS_START

namespace Steam
{
	bool RemoteStorage::FileWrite(const char *pchFile, const void *pvData, int cubData)
	{
		return true;
	}

	int RemoteStorage::GetFileSize(const char *pchFile)
	{
		return 0;
	}

	int RemoteStorage::FileRead(const char *pchFile, void *pvData, int cubDataToRead)
	{
#ifdef _DEBUG
		OutputDebugStringA(pchFile);
#endif
		return 0;
	}

	bool RemoteStorage::FileExists(const char *pchFile)
	{
		return false;
	}

	int RemoteStorage::GetFileCount()
	{
		return 0;
	}

	const char *RemoteStorage::GetFileNameAndSize(int iFile, int *pnFileSizeInBytes)
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
}

STEAM_IGNORE_WARNINGS_END
