#pragma once

namespace Components
{
	class Changelog : public Component
	{
	public:
		Changelog();

		static void SetChangelog(const std::string& changelog);

	private:
		static std::mutex Mutex;
		static std::vector<std::string> Lines;

		static unsigned int GetChangelogCount();
		static const char* GetChangelogText(unsigned int item, int column);
		static void SelectChangelog(unsigned int index);
	};
}
