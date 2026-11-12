#include "System.hpp"

namespace Game
{
  // Note that x64 folds this function and FS_FreeFileList() into one.
  //
  Sys_FreeFileList_t Sys_FreeFileList = Sys_FreeFileList_t(0x140277530);
  Sys_IsDatabaseReady_t Sys_IsDatabaseReady = Sys_IsDatabaseReady_t(0x140208940);
  Sys_IsDatabaseReady2_t Sys_IsDatabaseReady2 = Sys_IsDatabaseReady2_t(0x140208DE0);
  Sys_IsMainThread_t Sys_IsMainThread = Sys_IsMainThread_t(0x140208E10);
  Sys_IsRenderThread_t Sys_IsRenderThread = Sys_IsRenderThread_t(0x140208E60);
  Sys_IsServerThread_t Sys_IsServerThread = Sys_IsServerThread_t(0x140208EA0);
  Sys_IsDatabaseThread_t Sys_IsDatabaseThread = Sys_IsDatabaseThread_t(0x140209CB0);
  Sys_ListFiles_t Sys_ListFiles = Sys_ListFiles_t(0x14028C930);
  Sys_Milliseconds_t Sys_Milliseconds = Sys_Milliseconds_t(0x1402A6D20);
  Sys_Error_t Sys_Error = Sys_Error_t(0x1402A3480);
  Sys_Sleep_t Sys_Sleep = Sys_Sleep_t(0x1402A66A0);
  Sys_TempPriorityAtLeastNormalBegin_t Sys_TempPriorityAtLeastNormalBegin = Sys_TempPriorityAtLeastNormalBegin_t(0x140209590);
  Sys_TempPriorityEnd_t Sys_TempPriorityEnd = Sys_TempPriorityEnd_t(0x1402095D0);
  Sys_EnterCriticalSection_t Sys_EnterCriticalSection = Sys_EnterCriticalSection_t(0x14028C8B0);
  Sys_LeaveCriticalSection_t Sys_LeaveCriticalSection = Sys_LeaveCriticalSection_t(0x14028C910);
  Sys_DefaultInstallPath_t Sys_DefaultInstallPath = Sys_DefaultInstallPath_t(0x14028C750);
  Sys_SendPacket_t Sys_SendPacket = Sys_SendPacket_t(0x140205120);
  Sys_ShowConsole_t Sys_ShowConsole = Sys_ShowConsole_t(0x1402A7F30);
  Sys_SetErrorText_t Sys_SetErrorText = Sys_SetErrorText_t(0x1402A7EE0);
  Sys_Print_t Sys_Print = Sys_Print_t(0x1402A78C0);
  Sys_SuspendOtherThreads_t Sys_SuspendOtherThreads = Sys_SuspendOtherThreads_t(0x1402094C0);
  Sys_SetValue_t Sys_SetValue = Sys_SetValue_t(0x140209080);
  Sys_CreateFile_t Sys_CreateFile = Sys_CreateFile_t(0x14028C3C0);
  Sys_OutOfMemErrorInternal_t Sys_OutOfMemErrorInternal = Sys_OutOfMemErrorInternal_t(0x1402A3C60);
  Sys_QuitAndStartProcess_t Sys_QuitAndStartProcess = Sys_QuitAndStartProcess_t(0x1402A3E10);
  Sys_Quit_t Sys_Quit = Sys_Quit_t(0x1402A3DA0);
  Sys_QueEvent_t Sys_QueEvent = Sys_QueEvent_t(0x1402A3CC0);

  char(*sys_exitCmdLine)[1024] = reinterpret_cast<char(*)[1024]>(0x146789770);
  char(*sys_cmdline)[1024] = reinterpret_cast<char(*)[1024]>(0x146789B70);

  RTL_CRITICAL_SECTION* s_criticalSection = reinterpret_cast<RTL_CRITICAL_SECTION*>(0x14673CC20);

  void Sys_QueEvents(int time, int type, int value2, int ptrLength, void* ptr)
  {
    Sys_QueEvent(time, type, value2, TRUE, ptrLength, ptr);
    Sys_QueEvent(time, type, value2, FALSE, ptrLength, ptr);
  }

  void Sys_LockRead(FastCriticalSection* critSect)
  {
    InterlockedIncrement(&critSect->readCount);
    while (critSect->writeCount) Sys_Sleep(1);
  }

  // Note that x64 inlines this lock.
  //
  void Sys_LockWrite(FastCriticalSection* critSect)
  {
    TempPriority tempPriority;
    Sys_TempPriorityAtLeastNormalBegin(&tempPriority);

    for (;;)
    {
      if (critSect->readCount == 0)
      {
        if (InterlockedIncrement(&critSect->writeCount) == 1 && critSect->readCount == 0)
        {
          break;
        }

        InterlockedDecrement(&critSect->writeCount);
      }

      Sys_Sleep(1);
    }

    critSect->tempPriority = tempPriority;
  }

  void Sys_UnlockRead(FastCriticalSection* critSect)
  {
    assert(critSect->readCount > 0);
    InterlockedDecrement(&critSect->readCount);
  }

  void Sys_UnlockWrite(FastCriticalSection* critSect)
  {
    assert(critSect->writeCount > 0);
    InterlockedDecrement(&critSect->writeCount);
    Sys_TempPriorityEnd(&critSect->tempPriority);
  }

  bool Sys_TryEnterCriticalSection(CriticalSection critSect)
  {
    AssertIn(critSect, CRITSECT_COUNT);

    return TryEnterCriticalSection(&s_criticalSection[critSect]) != FALSE;
  }

  HANDLE Sys_OpenFileReliable(const char* filename)
  {
    return CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
      FILE_FLAG_OVERLAPPED | FILE_FLAG_NO_BUFFERING, nullptr);
  }
}
