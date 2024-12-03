#pragma once

namespace Components
{
	class Discord : public Component
	{
	public:
		Discord();

		static std::string GetDiscordServerLink() { return "https://discord.gg/2ETE8engZM"; }

		void preDestroy() override;

	private:
		static bool Initialized_;

		static void UpdateDiscord();
	};
}
