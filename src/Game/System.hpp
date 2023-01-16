#pragma once

namespace Game
{
	typedef void(*Sys_Error_t)(const char* error, ...);
	extern Sys_Error_t Sys_Error;

	typedef void(*Sys_FreeFileList_t)(const char** list);
	extern Sys_FreeFileList_t Sys_FreeFileList;

	typedef int(*Sys_IsDatabaseReady_t)();
	extern Sys_IsDatabaseReady_t Sys_IsDatabaseReady;

	typedef int(*Sys_IsDatabaseReady2_t)();
	extern Sys_IsDatabaseReady2_t Sys_IsDatabaseReady2;

	typedef bool(*Sys_IsMainThread_t)();
	extern Sys_IsMainThread_t Sys_IsMainThread;

	typedef bool(*Sys_IsRenderThread_t)();
	extern Sys_IsRenderThread_t Sys_IsRenderThread;

	typedef bool(*Sys_IsServerThread_t)();
	extern Sys_IsServerThread_t Sys_IsServerThread;

	typedef bool(*Sys_IsDatabaseThread_t)();
	extern Sys_IsDatabaseThread_t Sys_IsDatabaseThread;

	typedef const char**(*Sys_ListFiles_t)(const char* directory, const char* extension, const char* filter, int* numfiles, int wantsubs);
	extern Sys_ListFiles_t Sys_ListFiles;

	typedef int(*Sys_Milliseconds_t)();
	extern Sys_Milliseconds_t Sys_Milliseconds;

	typedef void(*Sys_Sleep_t)(int msec);
	extern Sys_Sleep_t Sys_Sleep;

	typedef void(*Sys_LockWrite_t)(FastCriticalSection* critSect);
	extern Sys_LockWrite_t Sys_LockWrite;

	typedef void(*Sys_TempPriorityAtLeastNormalBegin_t)(TempPriority*);
	extern Sys_TempPriorityAtLeastNormalBegin_t Sys_TempPriorityAtLeastNormalBegin;

	typedef void(*Sys_TempPriorityEnd_t)(TempPriority*);
	extern Sys_TempPriorityEnd_t Sys_TempPriorityEnd;

	typedef void(*Sys_EnterCriticalSection_t)(CriticalSection critSect);
	extern Sys_EnterCriticalSection_t Sys_EnterCriticalSection;

	typedef void(*Sys_LeaveCriticalSection_t)(CriticalSection critSect);
	extern Sys_LeaveCriticalSection_t Sys_LeaveCriticalSection;

	typedef char*(*Sys_DefaultInstallPath_t)();
	extern Sys_DefaultInstallPath_t Sys_DefaultInstallPath;

	typedef bool(*Sys_SendPacket_t)(netsrc_t sock, size_t len, const char* format, netadr_t adr);
	extern Sys_SendPacket_t Sys_SendPacket;

	typedef void(*Sys_ShowConsole_t)();
	extern Sys_ShowConsole_t Sys_ShowConsole;

	typedef void(*Sys_SuspendOtherThreads_t)();
	extern Sys_SuspendOtherThreads_t Sys_SuspendOtherThreads;

	typedef void(*Sys_SetValue_t)(int valueIndex, void* data);
	extern Sys_SetValue_t Sys_SetValue;

	typedef Sys_File(*Sys_CreateFile_t)(const char* dir, const char* filename);
	extern Sys_CreateFile_t Sys_CreateFile;

	typedef void(*Sys_OutOfMemErrorInternal_t)(const char* filename, int line);
	extern Sys_OutOfMemErrorInternal_t Sys_OutOfMemErrorInternal;

	extern char(*sys_exitCmdLine)[1024];

	extern RTL_CRITICAL_SECTION* s_criticalSection;

	extern void Sys_LockRead(FastCriticalSection* critSect);
	extern void Sys_UnlockRead(FastCriticalSection* critSect);
	extern void Sys_UnlockWrite(FastCriticalSection* critSect);

	extern bool Sys_TryEnterCriticalSection(CriticalSection critSect);

	class Sys
	{
	public:
		enum class TLS_OFFSET : unsigned int
		{
			LEVEL_BGS = 0xC,
			THREAD_VALUES = 0x14,
			HUFFMAN_BLOC = 0x10,
			DVAR_ALLOWED_MODIFIED_FLAGS = 0x1C,
			DVAR_MODIFIED_FLAGS = 0x18,
			ZIP_INFO = 0x20,
		};

		template <typename T>
		static T* GetTls(TLS_OFFSET offset)
		{
			const auto* tls = reinterpret_cast<std::uintptr_t*>(__readfsdword(0x2C));
			return reinterpret_cast<T*>(tls[*g_dwTlsIndex] + static_cast<std::underlying_type_t<TLS_OFFSET>>(offset));
		}
	};
}

#define Sys_OutOfMemError() Game::Sys_OutOfMemErrorInternal(__FILE__, __LINE__);
