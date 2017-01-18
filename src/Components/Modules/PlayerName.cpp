#include "STDInclude.hpp"

namespace Components
{
	std::string PlayerName::PlayerNames[18];

	int PlayerName::GetClientName(int /*localClientNum*/, int index, char *buf, int size)
	{
		if (index < 0 || index >= 18) return 0;
		return strncpy_s(buf, size, PlayerName::PlayerNames[index].data(), PlayerName::PlayerNames[index].size()) == 0;
	}

	PlayerName::PlayerName()
	{
		if (0) // Disabled for now (comment out that line to enable it)
		{
			for (int i = 0; i < ARRAY_SIZE(PlayerName::PlayerNames); ++i)
			{
				PlayerName::PlayerNames[i] = "mumu";
			}

			Utils::Hook(Game::CL_GetClientName, PlayerName::GetClientName, HOOK_JUMP).install()->quick();
		}
	}

	PlayerName::~PlayerName()
	{
		for (int i = 0; i < ARRAY_SIZE(PlayerName::PlayerNames); ++i)
		{
			PlayerName::PlayerNames[i].clear();
		}
	}
}
