#include "STDInclude.hpp"

namespace Components
{
	std::vector<Changelog::changelogLine> Changelog::Line;

	void Changelog::LoadChangelog(UIScript::Token)
	{
		if (!Changelog::Line.empty())
			return;

		std::vector<std::string> changelog;
		Changelog::Line.clear();

		{
			std::string uncleaned = Utils::Cache::GetFile("/iw4/changelog.txt");

			if (uncleaned.empty())
			{
				uncleaned = "^1Unable to get changelog.";
			}

			changelog = Utils::String::Explode(uncleaned, '\n');
		}

		for (auto line : changelog)
		{
			Changelog::changelogLine info;

			Utils::String::Replace(line, "\r", "");
			info.line = line;

			Changelog::Line.push_back(info);
		}
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
