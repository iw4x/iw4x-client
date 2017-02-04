#include "STDInclude.hpp"

namespace Components
{
	std::vector<Changelog::changelogLine> Changelog::Line;

	void Changelog::LoadChangelog(UIScript::Token)
	{
		std::vector<std::string> changelog;
		Changelog::Line.clear();

		{
			std::string uncleaned = Utils::Cache::GetFile("/iw4/changelog.txt");

			changelog = Utils::String::Explode(uncleaned, '\n');
		}

		for (auto line : changelog)
		{
			Changelog::changelogLine info;

			Utils::String::Replace(line, "\r", "");
			info.line = Utils::String::Trim(line);

			Changelog::Line.push_back(info);
		}

		// Reverse, latest demo first!
		//std::reverse(Changelog::Line.begin(), Changelog::Line.end());
	}

	unsigned int Changelog::GetChangelogCount()
	{
		return Changelog::Line.size();
	}

	// Omit column here
	const char* Changelog::GetChangelogText(unsigned int item, int /*column*/)
	{
		if (item < Changelog::Line.size())
		{
			std::string info = Changelog::Line[item].line;

			return Utils::String::VA("%s", info.data());
		}

		return "";
	}

	void Changelog::SelectChangelog(unsigned int index) { index; }

	Changelog::Changelog()
	{
		// UIScripts
		UIScript::Add("loadChangelog", Changelog::LoadChangelog);

		// Changelog
		UIFeeder::Add(39.0f, Changelog::GetChangelogCount, Changelog::GetChangelogText, Changelog::SelectChangelog);
	}
}
