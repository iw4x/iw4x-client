#include <STDInclude.hpp>

namespace Components
{
	Game::newMapArena_t ArenaLength::NewArenas[128];

	__declspec(naked) void ArenaLength::ArenaMapOffsetHook1()
	{
		__asm
		{
			lea eax, [esi + Game::newMapArena_t::mapName]
			push eax
			push ebx

			push 420725h
			retn
		}
	}

	__declspec(naked) void ArenaLength::ArenaMapOffsetHook2()
	{
		__asm
		{
			lea eax, [edi + Game::newMapArena_t::mapName]
			push eax
			push edx

			push 49BD3Eh
			retn
		}
	}

	__declspec(naked) void ArenaLength::ArenaMapOffsetHook3()
	{
		__asm
		{
			lea eax, [esi + Game::newMapArena_t::mapName]
			push eax
			push edx

			push 63279Eh
			retn
		}
	}

	__declspec(naked) void ArenaLength::ArenaMapOffsetHook4()
	{
		__asm
		{
			lea edi, [esi - Game::newMapArena_t::mapName]
			lea edx, [eax + 1]

			push 4064B8h
			retn
		}
	}

	ArenaLength::ArenaLength()
	{
		// Reallocate array
		Utils::Hook::Set<Game::newMapArena_t*>(0x417807, &ArenaLength::NewArenas[0]);
		Utils::Hook::Set<Game::newMapArena_t*>(0x420717, &ArenaLength::NewArenas[0]);
		Utils::Hook::Set<Game::newMapArena_t*>(0x49BD22, &ArenaLength::NewArenas[0]);
		Utils::Hook::Set<Game::newMapArena_t*>(0x4A9649, &ArenaLength::NewArenas[0]);
		Utils::Hook::Set<Game::newMapArena_t*>(0x4A97C2, &ArenaLength::NewArenas[0]);
		Utils::Hook::Set<Game::newMapArena_t*>(0x4D077E, &ArenaLength::NewArenas[0]);
		Utils::Hook::Set<Game::newMapArena_t*>(0x630B00, &ArenaLength::NewArenas[0]);
		Utils::Hook::Set<Game::newMapArena_t*>(0x630B2E, &ArenaLength::NewArenas[0]);
		Utils::Hook::Set<Game::newMapArena_t*>(0x632782, &ArenaLength::NewArenas[0]);

		Utils::Hook::Set<char*>(0x4A967A, ArenaLength::NewArenas[0].description);
		Utils::Hook::Set<char*>(0x4A96AD, ArenaLength::NewArenas[0].mapimage);

		Utils::Hook::Set<char*>(0x4A9616, ArenaLength::NewArenas[0].mapName);
		Utils::Hook::Set<char*>(0x4A9703, ArenaLength::NewArenas[0].mapName);
		Utils::Hook::Set<char*>(0x4064A8, ArenaLength::NewArenas[0].mapName);

		Utils::Hook::Set<char*>(0x42F214, &ArenaLength::NewArenas[0].other[0]);

		Utils::Hook::Set<char*>(0x4A96ED, &ArenaLength::NewArenas[0].other[0x8]);
		Utils::Hook::Set<char*>(0x4A9769, &ArenaLength::NewArenas[0].other[0x8]);
		Utils::Hook::Set<char*>(0x4A97A5, &ArenaLength::NewArenas[0].other[0x8]);

		Utils::Hook::Set<char*>(0x631E92, &ArenaLength::NewArenas[0].other[0x8C]);

		// Resize the array
		Utils::Hook::Set<int>(0x4064DE, sizeof(Game::newMapArena_t));
		Utils::Hook::Set<int>(0x417802, sizeof(Game::newMapArena_t));
		Utils::Hook::Set<int>(0x420736, sizeof(Game::newMapArena_t));
		Utils::Hook::Set<int>(0x42F271, sizeof(Game::newMapArena_t));
		Utils::Hook::Set<int>(0x49BD4F, sizeof(Game::newMapArena_t));
		Utils::Hook::Set<int>(0x4A960C, sizeof(Game::newMapArena_t));
		Utils::Hook::Set<int>(0x4A963F, sizeof(Game::newMapArena_t));
		Utils::Hook::Set<int>(0x4A9675, sizeof(Game::newMapArena_t));
		Utils::Hook::Set<int>(0x4A96A3, sizeof(Game::newMapArena_t));
		Utils::Hook::Set<int>(0x4A96E7, sizeof(Game::newMapArena_t));
		Utils::Hook::Set<int>(0x4A96FD, sizeof(Game::newMapArena_t));
		Utils::Hook::Set<int>(0x4A975A, sizeof(Game::newMapArena_t));
		Utils::Hook::Set<int>(0x4A979F, sizeof(Game::newMapArena_t));
		Utils::Hook::Set<int>(0x4A97BC, sizeof(Game::newMapArena_t));
		Utils::Hook::Set<int>(0x4D0779, sizeof(Game::newMapArena_t));
		Utils::Hook::Set<int>(0x630B29, sizeof(Game::newMapArena_t));
		Utils::Hook::Set<int>(0x630B81, sizeof(Game::newMapArena_t));
		Utils::Hook::Set<int>(0x631EBC, sizeof(Game::newMapArena_t));
		Utils::Hook::Set<int>(0x6327AF, sizeof(Game::newMapArena_t));

		Utils::Hook(0x420720, ArenaLength::ArenaMapOffsetHook1, HOOK_JUMP).install()->quick();
		Utils::Hook(0x49BD39, ArenaLength::ArenaMapOffsetHook2, HOOK_JUMP).install()->quick();
		Utils::Hook(0x632799, ArenaLength::ArenaMapOffsetHook3, HOOK_JUMP).install()->quick();
		Utils::Hook(0x4064B2, ArenaLength::ArenaMapOffsetHook4, HOOK_JUMP).install()->quick();

		Utils::Hook::Set<BYTE>(0x4A95F8, 32);

		Utils::Hook::Set<int>(0x42F22B, offsetof(Game::newMapArena_t, mapName) - offsetof(Game::newMapArena_t, other));

		// patch max arena count
		constexpr auto arenaCount = sizeof(ArenaLength::NewArenas) / sizeof(Game::newMapArena_t);
		Utils::Hook::Set<std::uint32_t>(0x630AA3, arenaCount);
	}
}
