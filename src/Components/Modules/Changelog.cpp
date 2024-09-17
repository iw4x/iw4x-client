#include <STDInclude.hpp>
#include "Changelog.hpp"
#include "UIFeeder.hpp"

namespace Components
{
	std::mutex Changelog::Mutex;
	std::vector<std::string> Changelog::Lines;

	void Changelog::LoadChangelog()
	{
		std::lock_guard _(Mutex);
		Lines.clear();

		const auto data = Utils::Cache::GetFile("/develop/CHANGELOG.md");

		if (data.empty())
		{
			Lines.emplace_back("^1Unable to get changelog.");
			return;
		}

		auto buffer = Utils::String::Split(data, '\n');
		for (auto& line : buffer)
		{
			Utils::String::Replace(line, "\r", "");
		}

		Lines = buffer;
	}

	unsigned int Changelog::GetChangelogCount()
	{
		return Lines.size();
	}

	// Omit column here
	const char* Changelog::GetChangelogText(unsigned int item, [[maybe_unused]] int column)
	{
		std::lock_guard _(Mutex);
		if (item < Lines.size())
		{
			return Utils::String::Format("{}", Lines[item]);
		}

		return "";
	}

	void Changelog::SelectChangelog([[maybe_unused]] unsigned int index)
	{
		// Don't do anything in here
	}

	Changelog::Changelog()
	{
		if (Dedicated::IsEnabled()) return;

		// Changelog
		UIFeeder::Add(62.0f, GetChangelogCount, GetChangelogText, SelectChangelog);
	}
}
