#include "STDInclude.hpp"

namespace Components
{
	std::vector<std::string> Flags::EnabledFlags;

	bool Flags::HasFlag(std::string flag)
	{
		for (auto entry : Flags::EnabledFlags)
		{
			if (Utils::StrToLower(entry) == Utils::StrToLower(flag))
			{
				return true;
			}
		}

		return false;
	}

	void Flags::ParseFlags()
	{
		int numArgs;
		LPCWSTR commandLine = GetCommandLineW();
		LPWSTR* argv = CommandLineToArgvW(commandLine, &numArgs);

		for (int i = 0; i < numArgs; ++i)
		{
			std::wstring wFlag = argv[i];
			if (wFlag[0] == L'-')
			{
				Flags::EnabledFlags.push_back(std::string(++wFlag.begin(), wFlag.end()));
			}
		}
	}

	Flags::Flags()
	{
		Flags::ParseFlags();
	}

	Flags::~Flags()
	{
		Flags::EnabledFlags.clear();
	}
}
