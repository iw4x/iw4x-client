#include <STDInclude.hpp>

namespace Game
{
	FS_FileExists_t FS_FileExists = FS_FileExists_t(0x4DEFA0);
	FS_FreeFile_t FS_FreeFile = FS_FreeFile_t(0x4416B0);
	FS_ReadFile_t FS_ReadFile = FS_ReadFile_t(0x4F4B90);
	FS_ListFiles_t FS_ListFiles = FS_ListFiles_t(0x441BB0);
	FS_FreeFileList_t FS_FreeFileList = FS_FreeFileList_t(0x4A5DE0);
	FS_FOpenFileAppend_t FS_FOpenFileAppend = FS_FOpenFileAppend_t(0x410BB0);
	FS_FOpenFileWrite_t FS_FOpenFileWrite = FS_FOpenFileWrite_t(0x4BA530);
	FS_FOpenTextFileWrite_t FS_FOpenTextFileWrite = FS_FOpenTextFileWrite_t(0x43FD90);
	FS_FOpenFileRead_t FS_FOpenFileRead = FS_FOpenFileRead_t(0x46CBF0);
	FS_FOpenFileReadDatabase_t FS_FOpenFileReadDatabase = FS_FOpenFileReadDatabase_t(0x42ECA0);
	FS_FOpenFileReadForThread_t FS_FOpenFileReadForThread = FS_FOpenFileReadForThread_t(0x643270);
	FS_FOpenFileByMode_t FS_FOpenFileByMode = FS_FOpenFileByMode_t(0x4C0700);
	FS_FCloseFile_t FS_FCloseFile = FS_FCloseFile_t(0x462000);
	FS_WriteFile_t FS_WriteFile = FS_WriteFile_t(0x426450);
	FS_WriteToDemo_t  FS_WriteToDemo = FS_WriteToDemo_t(0x4C06E0);
	FS_Write_t FS_Write = FS_Write_t(0x4576C0);
	FS_Printf_t FS_Printf = FS_Printf_t(0x459320);
	FS_Read_t FS_Read = FS_Read_t(0x4A04C0);
	FS_Seek_t FS_Seek = FS_Seek_t(0x4A63D0);
	FS_FTell_t FS_FTell = FS_FTell_t(0x4E6760);
	FS_Remove_t FS_Remove = FS_Remove_t(0x4660F0);
	FS_Restart_t FS_Restart = FS_Restart_t(0x461A50);
	FS_BuildPathToFile_t FS_BuildPathToFile = FS_BuildPathToFile_t(0x4702C0);
	FS_IsShippedIWD_t FS_IsShippedIWD = FS_IsShippedIWD_t(0x642440);
	FS_Delete_t FS_Delete = FS_Delete_t(0x48A5B0);
	FS_BuildOSPath_t FS_BuildOSPath = FS_BuildOSPath_t(0x4702C0);

	searchpath_s** fs_searchpaths = reinterpret_cast<searchpath_s**>(0x63D96E0);

	int FS_FOpenFileReadCurrentThread(const char* filename, int* file)
	{
		if (GetCurrentThreadId() == *reinterpret_cast<DWORD*>(0x1CDE7FC))
		{
			return FS_FOpenFileRead(filename, file);
		}

		if (GetCurrentThreadId() == *reinterpret_cast<DWORD*>(0x1CDE814))
		{
			return FS_FOpenFileReadDatabase(filename, file);
		}

		*file = 0;
		return -1;
	}

	void FS_AddLocalizedGameDirectory(const char* path, const char* dir)
	{
		static DWORD FS_AddLocalizedGameDirectory_t = 0x642EF0;

		__asm
		{
			pushad
			mov ebx, path
			mov eax, dir
			call FS_AddLocalizedGameDirectory_t
			popad
		}
	}
}
