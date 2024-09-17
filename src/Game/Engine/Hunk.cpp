#include <STDInclude.hpp>
#include "Hunk.hpp"

namespace Game::Engine
{
	HunkUser* g_debugUser;

	HunkUser* Hunk_UserCreate(int maxSize, const char* name, bool fixed, int type)
	{
		assert((!(maxSize % (64 * 1024))));

		auto* user = static_cast<HunkUser*>(Z_VirtualReserve(maxSize));
		Z_VirtualCommit(user, sizeof(HunkUser) - 4);

		user->end = reinterpret_cast<int>(user->buf) + maxSize - (sizeof(HunkUser) - 4);
		user->pos = reinterpret_cast<int>(user->buf);

		assert((!(user->pos & 31)));

		user->maxSize = maxSize;
		user->name = name;
		user->current = user;
		user->fixed = fixed;
		user->type = type;

		assert(!user->next);

		return user;
	}

	void Hunk_UserDestroy(HunkUser* user)
	{
		auto* current = user->next;
		while (current)
		{
			auto* next = current->next;
			Z_VirtualFree(current);
			current = next;
		}

		Z_VirtualFree(user);
	}

	void Hunk_InitDebugMemory()
	{
		assert(Sys_IsMainThread());
		assert(!g_debugUser);
		g_debugUser = Hunk_UserCreate(0x1000000, "Hunk_InitDebugMemory", false, 0);
	}

	void Hunk_ShutdownDebugMemory()
	{
		assert(Sys_IsMainThread());
		assert(g_debugUser);
		Hunk_UserDestroy(g_debugUser);
		g_debugUser = nullptr;
	}

	void* Hunk_AllocDebugMem(int size)
	{
		assert(Sys_IsMainThread());
		assert(g_debugUser);
		return Hunk_UserAlloc(g_debugUser, size, 4);
	}

	void Hunk_FreeDebugMem([[maybe_unused]] void* ptr)
	{
		assert(Sys_IsMainThread());
		assert(g_debugUser);

		// Let's hope it gets cleared by Hunk_ShutdownDebugMemory
	}
}
