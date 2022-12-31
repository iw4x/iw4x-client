#include <STDInclude.hpp>

namespace Components
{
	std::vector<std::string> Flags::EnabledFlags;

	bool Flags::HasFlag(const std::string& flag)
	{
		ParseFlags();

		for (const auto& entry : EnabledFlags)
		{
			if (Utils::String::ToLower(entry) == Utils::String::ToLower(flag))
			{
				return true;
			}
		}

		return false;
	}

	void Flags::ParseFlags()
	{
		static auto flagsParsed = false;
		if (flagsParsed)
		{
			return;
		}

		// Only parse flags once
		flagsParsed = true;
		int numArgs;
		auto* const argv = CommandLineToArgvW(GetCommandLineW(), &numArgs);

		if (argv)
		{
			for (auto i = 0; i < numArgs; ++i)
			{
				std::wstring wFlag(argv[i]);
				if (wFlag[0] == L'-')
				{
					wFlag.erase(wFlag.begin());
					EnabledFlags.emplace_back(Utils::String::Convert(wFlag));
				}
			}

			LocalFree(argv);
		}

		// Workaround for wine
		if (Utils::IsWineEnvironment() && Dedicated::IsEnabled() && !HasFlag("console") && !HasFlag("stdout"))
		{
			EnabledFlags.emplace_back("stdout");
		}
	}
}
