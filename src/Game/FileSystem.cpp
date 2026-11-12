#include "FileSystem.hpp"

namespace Game
{
  FS_FileExists_t FS_FileExists = FS_FileExists_t(0x1402750C0);
  FS_FreeFile_t FS_FreeFile = FS_FreeFile_t(0x140277520);
  FS_ReadFile_t FS_ReadFile = FS_ReadFile_t(0x140277570);
  FS_ListFiles_t FS_ListFiles = FS_ListFiles_t(0x140277540);
  FS_FreeFileList_t FS_FreeFileList = FS_FreeFileList_t(0x140277530);
  FS_FOpenFileWrite_t FS_FOpenFileWrite = FS_FOpenFileWrite_t(0x140274F30);
  FS_FileOpenReadText_t FS_FileOpenReadText = FS_FileOpenReadText_t(0x1401F6AD0);
  FS_FOpenFileRead_t FS_FOpenFileRead = FS_FOpenFileRead_t(0x1402740D0);
  FS_FOpenFileReadDatabase_t FS_FOpenFileReadDatabase = FS_FOpenFileReadDatabase_t(0x1402740F0);
  FS_FOpenFileReadForThread_t FS_FOpenFileReadForThread = FS_FOpenFileReadForThread_t(0x140274100);
  FS_FOpenFileByMode_t FS_FOpenFileByMode = FS_FOpenFileByMode_t(0x140273EE0);
  FS_FCloseFile_t FS_FCloseFile = FS_FCloseFile_t(0x140273E40);
  FS_WriteFile_t FS_WriteFile = FS_WriteFile_t(0x140277300);
  FS_WriteToDemo_t  FS_WriteToDemo = FS_WriteToDemo_t(0x140277370);
  FS_Write_t FS_Write = FS_Write_t(0x1402771C0);
  FS_Printf_t FS_Printf = FS_Printf_t(0x1402760F0);
  FS_Read_t FS_Read = FS_Read_t(0x1402761D0);
  FS_Seek_t FS_Seek = FS_Seek_t(0x140276810);
  FS_FTell_t FS_FTell = FS_FTell_t(0x140275090);
  FS_Remove_t FS_Remove = FS_Remove_t(0x14033A470);
  FS_Restart_t FS_Restart = FS_Restart_t(0x1402764D0);
  FS_IsShippedIWD_t FS_IsShippedIWD = FS_IsShippedIWD_t(0x140275AF0);
  FS_Delete_t FS_Delete = FS_Delete_t(0x140273D90);
  FS_BuildOSPath_t FS_BuildOSPath = FS_BuildOSPath_t(0x140273AD0);
  FS_AddLocalizedGameDirectory_t FS_AddLocalizedGameDirectory = FS_AddLocalizedGameDirectory_t(0x1402737B0);

  searchpath_s** fs_searchpaths = reinterpret_cast<searchpath_s**>(0x146642A10);

  char* fs_gamedir = reinterpret_cast<char*>(0x1466428E0);

  fileHandleData_t* fsh = reinterpret_cast<fileHandleData_t*>(0x146642A20);

  // Note that x64 only flushes inside FS_Write() for handles opened for
  // synchronous writes.
  //
  void FS_Flush(fileHandle_t f)
  {
    std::fflush(fsh[f].handleFiles.file.o);
  }

  int FS_FOpenFileReadCurrentThread(const char* filename, fileHandle_t* file)
  {
    if (GetCurrentThreadId() == *reinterpret_cast<DWORD*>(0x141DF0F90))
    {
      return FS_FOpenFileRead(filename, file);
    }

    if (GetCurrentThreadId() == *reinterpret_cast<DWORD*>(0x141DF0FA8))
    {
      return FS_FOpenFileReadDatabase(filename, file);
    }

    *file = 0;
    return -1;
  }
}
