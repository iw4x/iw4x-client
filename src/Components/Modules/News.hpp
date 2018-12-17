#pragma once

namespace Components
{
	class News : public Component
	{
	public:
		News();
		~News();

		void preDestroy() override;
		bool unitTest() override;

		static void LaunchUpdater(const std::string& params);
		static bool Updating();

	private:
		static std::string UpdaterArgs;
		static std::string UpdaterHash;
		static std::thread Thread;
		static std::mutex UpdaterMutex;

		static bool Terminate;
		static bool GetLatestUpdater();
		static bool DownloadUpdater();

		static void CheckForUpdate();
		static void ExitProcessStub(unsigned int exitCode);

		static const char* GetNewsText();
	};
}
