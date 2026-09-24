#include "ConnectProtocol.hpp"
#include "IPCPipe.hpp"

namespace Components
{
	namespace
	{
		LSTATUS SetRegistryString(const wchar_t* keyPath, const wchar_t* valueName, const std::wstring& value)
		{
			HKEY key = nullptr;
			const auto result = RegCreateKeyExW(HKEY_CURRENT_USER, keyPath, 0, nullptr,
				REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, nullptr, &key, nullptr);

			if (result != ERROR_SUCCESS)
			{
				return result;
			}

			const auto closeKey = gsl::finally([&key]
			{
				RegCloseKey(key);
			});

			const auto size = static_cast<DWORD>((value.size() + 1) * sizeof(wchar_t));
			return RegSetValueExW(key, valueName, 0, REG_SZ,
				reinterpret_cast<const BYTE*>(value.c_str()), size);
		}
	}

	bool ConnectProtocol::Evaluated = false;
	std::string ConnectProtocol::ConnectString;

	bool ConnectProtocol::IsEvaluated()
	{
		return Evaluated;
	}

	bool ConnectProtocol::Used()
	{
		if (!IsEvaluated())
		{
			EvaluateProtocol();
		}

		return (!ConnectString.empty());
	}

	bool ConnectProtocol::InstallProtocol()
	{
		wchar_t ownPath[MAX_PATH]{};
		const auto pathLength = GetModuleFileNameW(nullptr, ownPath, ARRAYSIZE(ownPath));

		if (pathLength == 0 || pathLength >= ARRAYSIZE(ownPath))
		{
			return false;
		}

		const std::filesystem::path executable(ownPath);
		const auto workdir = executable.parent_path();

		if (workdir.empty() || !SetCurrentDirectoryW(workdir.c_str()))
		{
			return false;
		}

		const auto quotedExecutable = L"\"" + executable.native() + L"\"";
		bool success = true;

		const auto set = [&success](const wchar_t* subkey, const wchar_t* name,
			const std::wstring& value, const char* description)
		{
			std::wstring key = L"SOFTWARE\\Classes\\iw4x";

			if (subkey != nullptr)
			{
				key += L'\\';
				key += subkey;
			}

			const auto result = SetRegistryString(key.c_str(), name, value);
			if (result == ERROR_SUCCESS)
			{
				return;
			}

			success = false;
			Logger::Warning(Game::CON_CHANNEL_SYSTEM,
				"Unable to register IW4x {}: {} ({})\n", description,
				std::system_category().message(result), result);
		};

		set(nullptr, nullptr, L"URL:IW4x Protocol", "protocol description");
		set(nullptr, L"URL Protocol", L"", "URL protocol marker");
		set(L"DefaultIcon", nullptr, quotedExecutable + L",0", "protocol icon");
		set(L"shell\\open\\command", nullptr, quotedExecutable + L" \"%1\"", "protocol command");

		return success;
	}

	void ConnectProtocol::EvaluateProtocol()
	{
		if (Evaluated) return;
		Evaluated = true;

		std::string cmdLine = GetCommandLineA();

		auto pos = cmdLine.find("iw4x://");

		if (pos != std::string::npos)
		{
			cmdLine = cmdLine.substr(pos + 7);
			pos = cmdLine.find_first_of('/');

			if (pos != std::string::npos)
			{
				cmdLine = cmdLine.substr(0, pos);
			}

			ConnectString = cmdLine;
		}
	}

	void ConnectProtocol::Invocation()
	{
		if (Used())
		{
			const auto* cmd = Utils::String::Format("connect {}", ConnectString);
			Command::Execute(cmd, false);
		}
	}

	ConnectProtocol::ConnectProtocol()
	{
		if (Dedicated::IsEnabled()) return;

		// IPC handler
		IPCPipe::On("connect", [](const std::string& data)
		{
			const auto* cmd = Utils::String::Format("connect {}", data);
			Command::Execute(cmd, false);
		});

		// Invocation handler
		Scheduler::OnGameInitialized(Invocation, Scheduler::Pipeline::MAIN);

		InstallProtocol();
		EvaluateProtocol();

		// Fire protocol handlers
		// Make sure this happens after the pipe-initialization!
		if (Used())
		{
			if (!Singleton::IsFirstInstance())
			{
				IPCPipe::Write("connect", ConnectString);
				ExitProcess(EXIT_SUCCESS);
			}
			else
			{
				// Only skip intro here, invocation will be done later.
				Utils::Hook::Set<std::uint8_t>(0x60BECF, 0xEB);

				Scheduler::Once([]
				{
					Command::Execute("openmenu popup_reconnectingtoparty", false);
				}, Scheduler::Pipeline::MAIN, 8s);
			}
		}
	}
}
