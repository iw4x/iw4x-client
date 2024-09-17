#pragma once

namespace Components
{
	class ArenaLength : public Component
	{
	public:
		ArenaLength();

		static Game::newMapArena_t NewArenas[];
		static char* NewArenaInfos[];

	private:
		static void ArenaMapOffsetHook1();
		static void ArenaMapOffsetHook2();
		static void ArenaMapOffsetHook3();
		static void ArenaMapOffsetHook4();
	};
}
