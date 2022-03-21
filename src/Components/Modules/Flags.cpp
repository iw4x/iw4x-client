#include <STDInclude.hpp>

namespace Components
{
	std::vector<std::string> Flags::EnabledFlags;

	bool Flags::HasFlag(const std::string& flag)
	{
		static auto parsed = false;

		if (!parsed)
		{
			Flags::ParseFlags();
			parsed = true;
		}

		for (const auto& entry : Flags::EnabledFlags)
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
		int numArgs;
		auto* const argv = CommandLineToArgvW(GetCommandLineW(), &numArgs);

		assert(Flags::EnabledFlags.empty());

		if (argv)
		{
			for (auto i = 0; i < numArgs; ++i)
			{
				std::wstring wFlag(argv[i]);
				if (wFlag[0] == L'-')
				{
					wFlag.erase(wFlag.begin());
					Flags::EnabledFlags.push_back(Utils::String::Convert(wFlag));
				}
			}

			LocalFree(argv);
		}

		// Workaround for wine
		if (Utils::IsWineEnvironment() && Dedicated::IsEnabled() && !Flags::HasFlag("console") && !Flags::HasFlag("stdout"))
		{
			Flags::EnabledFlags.push_back("stdout");
		}
	}
}
