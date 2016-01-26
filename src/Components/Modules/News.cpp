#include "STDInclude.hpp"

namespace Components
{
	std::thread* News::Thread = 0;

	News::News()
	{
		Localization::Set("MPUI_CHANGELOG_TEXT", "");
		Localization::Set("MPUI_MOTD_TEXT", "Welcome to ReactIW4x Multiplayer!");

		News::Thread = new std::thread([] ()
		{
			Localization::Set("MPUI_CHANGELOG_TEXT", Utils::WebIO("IW4x", "https://iw4xcachep26muba.onion.to/iw4/changelog.txt").Get().data());

			std::string data = Utils::WebIO("IW4x", "https://iw4xcachep26muba.onion.to/iw4/motd.txt").Get();

			if (data.size())
			{
				Localization::Set("MPUI_MOTD_TEXT", data.data());
			}

			// TODO: Implement update checks here!
		});
	}

	News::~News()
	{
		if (News::Thread)
		{
			News::Thread->join();
			delete News::Thread;
		}
	}
}
