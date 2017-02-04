#pragma once

namespace Components
{
	class Changelog : public Component
	{
	public:
		Changelog();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* getName() override { return "Changelog"; };
#endif

		class changelogLine
		{
		public:
			std::string line;
		};

	private:
		static std::vector<changelogLine> Line;

		static void LoadChangelog(UIScript::Token);

		static unsigned int GetChangelogCount();
		static const char* GetChangelogText(unsigned int item, int column);
		static void SelectChangelog(unsigned int index);
	};
}
