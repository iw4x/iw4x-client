#pragma once

namespace Game
{
	typedef void(*FS_FreeFile_t)(void* buffer);
	extern FS_FreeFile_t FS_FreeFile;

	typedef int(*FS_ReadFile_t)(const char* path, char** buffer);
	extern FS_ReadFile_t FS_ReadFile;

	typedef const char**(*FS_ListFiles_t)(const char* path, const char* extension, FsListBehavior_e behavior, int* numfiles, int allocTrackType);
	extern FS_ListFiles_t FS_ListFiles;

	typedef void(*FS_FreeFileList_t)(const char** list, int allocTrackType);
	extern FS_FreeFileList_t FS_FreeFileList;

	typedef int(*FS_FOpenFileAppend_t)(const char* filename);
	extern FS_FOpenFileAppend_t FS_FOpenFileAppend;

	typedef int(*FS_FOpenFileWrite_t)(const char* filename);
	extern FS_FOpenFileWrite_t FS_FOpenFileWrite;

	typedef int(*FS_FOpenTextFileWrite_t)(const char* filename);
	extern FS_FOpenTextFileWrite_t FS_FOpenTextFileWrite;

	typedef int(*FS_FOpenFileRead_t)(const char* filename, int* file);
	extern FS_FOpenFileRead_t FS_FOpenFileRead;

	typedef int(*FS_FOpenFileReadDatabase_t)(const char* filename, int* file);
	extern FS_FOpenFileReadDatabase_t FS_FOpenFileReadDatabase;

	typedef int(*FS_FOpenFileReadForThread_t)(const char* filename, int* file, int thread);
	extern FS_FOpenFileReadForThread_t FS_FOpenFileReadForThread;

	typedef int(*FS_FOpenFileByMode_t)(const char* qpath, int* f, fsMode_t mode);
	extern FS_FOpenFileByMode_t FS_FOpenFileByMode;

	typedef int(*FS_FCloseFile_t)(int stream);
	extern FS_FCloseFile_t FS_FCloseFile;

	typedef bool(*FS_FileExists_t)(const char* file);
	extern FS_FileExists_t FS_FileExists;

	typedef bool(*FS_WriteFile_t)(const char* filename, const char* folder, const void* buffer, int size);
	extern FS_WriteFile_t FS_WriteFile;

	typedef int(*FS_WriteToDemo_t)(const void* buffer, int size, int file);
	extern FS_WriteToDemo_t FS_WriteToDemo;

	typedef int(*FS_Write_t)(const void* buffer, int len, int h);
	extern FS_Write_t FS_Write;

	typedef int(*FS_Printf_t)(int file, const char* fmt, ...);
	extern FS_Printf_t FS_Printf;

	typedef int(*FS_Read_t)(void* buffer, int len, int h);
	extern FS_Read_t FS_Read;

	typedef int(*FS_Seek_t)(int fileHandle, int seekPosition, int seekOrigin);
	extern FS_Seek_t FS_Seek;

	typedef int(*FS_FTell_t)(int fileHandle);
	extern FS_FTell_t FS_FTell;

	typedef int(*FS_Remove_t)(char*);
	extern FS_Remove_t FS_Remove;

	typedef int(*FS_Restart_t)(int localClientNum, int checksumFeed);
	extern FS_Restart_t FS_Restart;

	typedef int(*FS_BuildPathToFile_t)(const char*, const char*, const char*, char**);
	extern FS_BuildPathToFile_t FS_BuildPathToFile;

	typedef iwd_t* (*FS_IsShippedIWD_t)(const char* fullpath, const char* iwd);
	extern FS_IsShippedIWD_t FS_IsShippedIWD;

	typedef int(__cdecl* FS_Delete_t)(const char* fileName);
	extern FS_Delete_t FS_Delete;

	typedef void(*FS_BuildOSPath_t)(const char* base, const char* game, const char* qpath, char* ospath);
	extern FS_BuildOSPath_t FS_BuildOSPath;

	extern searchpath_s** fs_searchpaths;

	extern int FS_FOpenFileReadCurrentThread(const char* filename, int* file);

	extern void FS_AddLocalizedGameDirectory(const char* path, const char* dir);
}
