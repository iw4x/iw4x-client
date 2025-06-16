#pragma once

namespace Components
{
	class Discord : public Component
	{
	public:
		Discord();

		static std::string GetDiscordServerLink() { return "https://iw4x.dev/discord"; }

		void preDestroy() override;

	private:
		static bool Initialized_;

		static void UpdateDiscord();
	};
}
