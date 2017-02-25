#pragma once

namespace Components
{
	class News : public Component
	{
	public:
		News();
		~News();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* getName() override { return "News"; };
#endif

		void preDestroy() override;
		bool unitTest() override;

		static void LaunchUpdater(std::string params);
		static bool Updating();

	private:
		static std::string UpdaterArgs;
		static std::string UpdaterHash;
		static std::thread Thread;
		
		static bool Terminate;
		static bool GetLatestUpdater();
		static bool DownloadUpdater();

		static void CheckForUpdate();
		static void ExitProcessStub(unsigned int exitCode);

		static const char* GetNewsText();
	};
}
