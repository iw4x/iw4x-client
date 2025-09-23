#pragma once

namespace Components
{
	class Discord
	{
	public:
		Discord();

		static std::string GetDiscordServerLink() { return "https://iw4x.io/discord"; }

	private:
		static bool Initialized_;

		static void UpdateDiscord();
	};
}
