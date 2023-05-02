#pragma once

namespace Components
{
	class Stats : public Component
	{
	public:
		Stats();

		static bool IsMaxLevel();

	private:
		static void UpdateClasses([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info);

		static void SendStats();
		static int SaveStats(char* dest, const char* folder, const char* buffer, int size);

		static std::int64_t* GetStatsID();

		static void AddScriptFunctions();
	};
}
