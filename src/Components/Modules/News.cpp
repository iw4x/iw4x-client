#include "STDInclude.hpp"

namespace Components
{
	std::string News::Motd;

	const char* News::GetMotd()
	{
		return News::Motd.data();
	}

	News::News()
	{
		Localization::Set("MPUI_CHANGELOG_TEXT", "");

		News::Motd = Utils::WebIO("IW4x", "http://localhost/iw4/motd.txt").SetTimeout(1000)->Get();

		if (!News::Motd.size())
		{
			News::Motd = "Welcome to ReactIW4x Multiplayer!";
		}

		Localization::Set("MPUI_CHANGELOG_TEXT", Utils::WebIO("IW4x", "http://localhost/iw4/changelog.txt").SetTimeout(1000)->Get().data());

		// Patch motd setting
		Utils::Hook(0x60BF19, News::GetMotd, HOOK_CALL).Install()->Quick();
	}
}
