#pragma once

#define FIXED_HUNK_USER_COUNT 1
#define VIRTUAL_HUNK_USER_MAX 128

namespace Game::Engine
{
	extern HunkUser* g_debugUser;

	extern HunkUser* Hunk_UserCreate(int maxSize, const char* name, bool fixed, int type);
	extern void Hunk_UserDestroy(HunkUser* user);

	extern void Hunk_InitDebugMemory();
	extern void Hunk_ShutdownDebugMemory();
	extern void* Hunk_AllocDebugMem(int size);
	extern void Hunk_FreeDebugMem(void* ptr);
}
