#pragma once

namespace Components
{
	class Changelog : public Component
	{
	public:
		Changelog();
		~Changelog();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* getName() override { return "Changelog"; };
#endif

		static void LoadChangelog();

	private:
		static std::mutex Mutex;
		static std::vector<std::string> Lines;

		static unsigned int GetChangelogCount();
		static const char* GetChangelogText(unsigned int item, int column);
		static void SelectChangelog(unsigned int index);
	};
}
