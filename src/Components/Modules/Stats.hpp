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

		static std::int64_t* GetStatsID();

		static void AddScriptFunctions();

		static void SprintfLiveStorageFilename(char* target, size_t size);
		static void SprintfLiveStorageFilenameWithFsGame(char* target, size_t size, const char* fsGame);
		
		static void MoveOldStatsToNewFolder();

		static uint32_t HashFilename();
		static void HashFilenameStub();
	};
}
