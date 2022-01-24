#pragma once

namespace Components
{
	class Bots : public Component
	{
	public:
		Bots();

		enum PlayerKeyFlag
		{
			FIRE = 0x1,
			SPRINT = 0x2,
			MELEE = 0x4,
			RELOAD = 0x10,
			LEAN_LEFT = 0x40,
			LEAN_RIGHT = 0x80,
			PRONE = 0x100,
			CROUCH = 0x200,
			JUMP = 0x400,
			ADS_MODE = 0x800,
			TEMP_ACTION = 0x1000,
			HOLD_BREATH = 0x2000,
			THROW_EQUIPMENT = 0x4000,
			THROW_SPECIAL = 0x8000,
			NIGHT_VISION = 0x40000,
			ADS = 0x80000,

			NUM_0 = 0x8,
			NUM_1 = 0x20,
			NUM_2 = 0x10000,
			NUM_3 = 0x20000,
			NUM_4 = 0x100000,
			NUM_5 = 0x200000,
			NUM_6 = 0x400000,
			NUM_7 = 0x800000,
			NUM_8 = 0x1000000,
			NUM_9 = 0x2000000,

			USE = NUM_0 | NUM_1
		};

	private:
		static std::vector<std::string> BotNames;

		static void BuildConnectString(char* buffer, const char* connectString, int num, int, int protocol, int checksum, int statVer, int statStuff, int port);

		static void Spawn(unsigned int count);

		static void AddMethods();

		static void BotAiAction();
		static void SV_UpdateBots_Hk();
	};
}
