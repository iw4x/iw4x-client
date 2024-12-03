#include <STDInclude.hpp>

#include "Console.hpp"
#include "Exception.hpp"
#include "Window.hpp"
#include "Party.hpp"
#include "TextRenderer.hpp"
#include "Discord.hpp"

#include <cwctype>
#include <version.hpp>

namespace Components
{
	constexpr auto CLIPBOARD_MSG = "Do you want to copy this message to the clipboard?";
	
	Utils::Hook Exception::SetFilterHook;
	int Exception::MiniDumpType;

	__declspec(noreturn) void Exception::LongJmp_Internal_Stub(jmp_buf env, int status)
	{
		AssetHandler::ResetBypassState();
		Game::longjmp_internal(env, status);
	}

	void Exception::SuspendProcess()
	{
		FreeConsole();

		if (IsWindow(Console::GetWindow()) != FALSE)
		{
			CloseWindow(Console::GetWindow());
			DestroyWindow(Console::GetWindow());
		}

		if (IsWindow(Window::GetWindow()) != FALSE)
		{
			CloseWindow(Window::GetWindow());
			DestroyWindow(Window::GetWindow());

			std::this_thread::sleep_for(2s);

			// This makes sure we either destroy the windows or wait till they are destroyed
			MSG msg;
			Utils::Time::Interval interval;
			while (IsWindow(Window::GetWindow()) != FALSE && !interval.elapsed(2s))
			{
				if (PeekMessageA(&msg, nullptr, NULL, NULL, PM_REMOVE))
				{
					TranslateMessage(&msg);
					DispatchMessageA(&msg);
				}

				std::this_thread::sleep_for(10ms);
			}
		}

		// This only suspends the main game threads, which is enough for us
		Game::Sys_SuspendOtherThreads();
	}

	std::wstring Exception::GetErrorMessage()
	{
		const auto clientVersion = (*Game::shortversion)->current.string;
		const auto osVersion = Utils::IsWineEnvironment() ? "Wine" : Utils::GetWindowsVersion();
		const auto launchParams = Utils::String::Convert(Utils::GetLaunchParameters());

		std::string clientInfo = std::format(R"(
			Client Info:
			IW4x Version: {}
			OS Version: {}
			Parameters: {})",
			clientVersion, osVersion, launchParams);

		if (!Game::CL_IsCgameInitialized())
		{
			std::string msg = std::format("{}\n\n{}", clientInfo, CLIPBOARD_MSG);
			std::wstring message(msg.begin(), msg.end());
			return message;
		}

		const auto* gameType = (*Game::sv_gametype)->current.string;
		const auto* mapName  = (*Game::sv_mapname)->current.string;
		const auto* fsGame   = (*Game::fs_gameDirVar)->current.string[0] != '\0' ? (*Game::fs_gameDirVar)->current.string : "None";

		char modName[256]{ 0 };
		TextRenderer::StripColors(fsGame, modName, sizeof(modName));
		TextRenderer::StripAllTextIcons(modName, modName, sizeof(modName));

		// Get info for a private match
		{
			if (Dedicated::IsRunning())
			{
				std::string privateMatchInfo = std::format(R"(
					Host Info:
					Type: Private Match
					Gametype: {}
					Map Name: {}
					Mod Name: {})",
					gameType, mapName, modName);

				std::string msg = std::format("{}\n{}\n\n{}", clientInfo, privateMatchInfo, CLIPBOARD_MSG);
				std::wstring message(msg.begin(), msg.end());
				return message;
			}
		}

		// Get info for a dedicated server
		{
			const auto serverVersion = Dvar::Var("sv_version").get<std::string>();
			const auto ipAddress = Network::Address(*Game::connectedHost).getString();

			char serverName[256]{ 0 };
			TextRenderer::StripColors(Party::GetHostName().data(), serverName, sizeof(serverName));
			TextRenderer::StripAllTextIcons(serverName, serverName, sizeof(serverName));

			std::string serverInfo = std::format(R"(
				Server Info:
				Type: Dedicated Server							
				IW4x Version: {}
				Server Name: {}
				IP Address: {}					
				Gametype: {}
				Map Name: {}
				Mod Name: {})",
				serverVersion, serverName, ipAddress, gameType, mapName, modName);

			std::string msg = std::format("{}\n{}\n\n{}", clientInfo, serverInfo, CLIPBOARD_MSG);
			std::wstring message(msg.begin(), msg.end());
			return message;
		}
	}

	std::string Exception::FormatMessageForClipboard(const std::wstring& message)
	{
		std::wstringstream ss(message);
		std::wstring line;
		std::wostringstream result;

		// Trim all the whitespaces from the message
		auto trim = [](std::wstring& s)
		{
			s.erase(s.begin(), std::find_if_not(s.begin(), s.end(), [](wchar_t ch) { return std::iswspace(ch); })); // trim left
			s.erase(std::find_if_not(s.rbegin(), s.rend(), [](wchar_t ch) { return std::iswspace(ch); }).base());	// trim right
		};

		// Construct a corrected version and get rid of the last line
		while (std::getline(ss, line))
		{
			trim(line);

			if (line != Utils::String::Convert(CLIPBOARD_MSG))
			{
				if (!line.empty())
				{
					result << line << L'\n';
				}
			}
		}

		// Enough wide strings for today
		return Utils::String::Convert(result.str());
	}

	void Exception::DisplayErrorMessage(const std::wstring& title, const std::wstring& message)
	{
		const std::wstring footerText = std::format(
			L"Join the official <a href=\"{}\">Discord Server</a> for additional support.",
			Utils::String::Convert(Discord::GetDiscordServerLink()));

		TASKDIALOGCONFIG taskDialogConfig = { 0 };
		taskDialogConfig.cbSize				= sizeof(taskDialogConfig);
		taskDialogConfig.hInstance			= GetModuleHandleA(nullptr);
		taskDialogConfig.hwndParent			= Window::GetWindow();
		taskDialogConfig.pszWindowTitle		= L"Unhandled Exception";
		taskDialogConfig.pszMainIcon		= MAKEINTRESOURCEW(-7); // Red bar with a shield icon
		taskDialogConfig.pszMainInstruction = title.c_str();
		taskDialogConfig.pszContent			= message.c_str();
		taskDialogConfig.dwCommonButtons	= TDCBF_YES_BUTTON | TDCBF_NO_BUTTON;
		taskDialogConfig.nDefaultButton		= IDYES;
		taskDialogConfig.pszFooterIcon		= TD_INFORMATION_ICON;
        taskDialogConfig.pszFooter			= footerText.c_str();
		taskDialogConfig.dwFlags			= TDF_ENABLE_HYPERLINKS | TDF_POSITION_RELATIVE_TO_WINDOW | TDF_SIZE_TO_CONTENT;
		taskDialogConfig.lpCallbackData		= reinterpret_cast<LONG_PTR>(&message);
		taskDialogConfig.pfCallback			= Exception::TaskDialogCallbackProc;

		::TaskDialogIndirect(&taskDialogConfig, nullptr, nullptr, nullptr);
	}

	HRESULT CALLBACK Exception::TaskDialogCallbackProc(HWND, UINT notification, WPARAM clickedButton, LPARAM, LONG_PTR data)
	{
		const auto* msg = reinterpret_cast<const std::wstring*>(data);

		if (notification == TDN_HYPERLINK_CLICKED)
		{
			Utils::OpenUrl(Discord::GetDiscordServerLink());
		}

		if (notification == TDN_BUTTON_CLICKED)
		{
			if (clickedButton == IDYES)
			{
				std::string formattedMessage = Exception::FormatMessageForClipboard(*msg);
				Exception::CopyMessageToClipboard(formattedMessage.c_str());
			}
		}

		return S_OK;
	}

	void Exception::CopyMessageToClipboard(const char* error)
	{
		const auto hWndNewOwner = GetDesktopWindow();
		const auto result = OpenClipboard(hWndNewOwner);

		if (result == FALSE)
		{
			return;
		}

		const auto _0 = gsl::finally([]
		{
			CloseClipboard();
		});

		EmptyClipboard();

		const auto len = std::strlen(error);
		auto* hMem = GlobalAlloc(GMEM_MOVEABLE, len + 1);

		if (!hMem)
		{
			return;
		}

		auto* lock = GlobalLock(hMem);
		if (lock)
		{
			std::memcpy(lock, error, len + 1);
			GlobalUnlock(hMem);
			SetClipboardData(CF_TEXT, hMem);
		}

		GlobalFree(hMem);
	}

	LONG WINAPI Exception::ExceptionFilter(LPEXCEPTION_POINTERS ExceptionInfo)
	{
		// Pass on harmless errors
		if (ExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_INTEGER_OVERFLOW ||
			ExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_FLOAT_OVERFLOW)
		{
			return EXCEPTION_CONTINUE_EXECUTION;
		}

		if (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_STACK_OVERFLOW)
		{
			const auto error = std::format("Termination because of a stack overflow.\n{}", CLIPBOARD_MSG);

			// Message should be copied to the clipboard if no button is pressed
			if (MessageBoxA(nullptr, error.c_str(), nullptr, MB_YESNO | MB_ICONERROR) == IDYES)
			{
				CopyMessageToClipboard(Utils::String::VA("0x%08X", ExceptionInfo->ExceptionRecord->ExceptionAddress));
			}
		}
		else
		{
			const auto code	   = ExceptionInfo->ExceptionRecord->ExceptionCode;
			const auto address = reinterpret_cast<std::uintptr_t>(ExceptionInfo->ExceptionRecord->ExceptionAddress);

			std::wstring title   = Utils::String::Convert(std::format("Fatal error (0x{:X}) at 0x{:X}", code, address));
			std::wstring message = Exception::GetErrorMessage();

			Exception::DisplayErrorMessage(title, message);	
		}

		if (Flags::HasFlag("bigminidumps"))
		{
			SetMiniDumpType(true, false);
		}

		// Current executable name
		char exeFileName[MAX_PATH];
		GetModuleFileNameA(nullptr, exeFileName, MAX_PATH);
		PathStripPathA(exeFileName);
		PathRemoveExtensionA(exeFileName);

		// Generate filename
		char filenameFriendlyTime[MAX_PATH]{};
		__time64_t time;
		tm ltime;
		_time64(&time);
		_localtime64_s(&ltime, &time);
		strftime(filenameFriendlyTime, sizeof(filenameFriendlyTime) - 1, "%Y%m%d%H%M%S", &ltime);

		// Combine with queued MinidumpsFolder
		char filename[MAX_PATH]{};
		CreateDirectoryA("minidumps", nullptr);
		PathCombineA(filename, "minidumps\\", Utils::String::VA("%s-" REVISION_STR "-%s.dmp", exeFileName, filenameFriendlyTime));

		constexpr auto fileShare = FILE_SHARE_READ | FILE_SHARE_WRITE;
		HANDLE hFile = CreateFileA(filename, GENERIC_WRITE | GENERIC_READ, fileShare, nullptr, (fileShare & FILE_SHARE_WRITE) > 0 ? OPEN_ALWAYS : OPEN_EXISTING, NULL, nullptr);
		MINIDUMP_EXCEPTION_INFORMATION ex = { GetCurrentThreadId(), ExceptionInfo, FALSE };
		if (!MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, static_cast<MINIDUMP_TYPE>(MiniDumpType), &ex, nullptr, nullptr))
		{
			MessageBoxA(nullptr, Utils::String::Format("There was an error creating the minidump ({})! Hit OK to close the program.", Utils::GetLastWindowsError()), "ERROR", MB_OK | MB_ICONERROR);
#ifdef _DEBUG
			OutputDebugStringA("Failed to create new minidump!");
			Utils::OutputDebugLastError();
#endif
			TerminateProcess(GetCurrentProcess(), ExceptionInfo->ExceptionRecord->ExceptionCode);
		}

		{
			TerminateProcess(GetCurrentProcess(), ExceptionInfo->ExceptionRecord->ExceptionCode);
		}

		return EXCEPTION_CONTINUE_SEARCH;
	}

	void Exception::SetMiniDumpType(bool codeseg, bool dataseg)
	{
		MiniDumpType = MiniDumpIgnoreInaccessibleMemory;
		MiniDumpType |= MiniDumpWithHandleData;
		MiniDumpType |= MiniDumpScanMemory;
		MiniDumpType |= MiniDumpWithProcessThreadData;
		MiniDumpType |= MiniDumpWithFullMemoryInfo;
		MiniDumpType |= MiniDumpWithThreadInfo;

		if (codeseg)
		{
			MiniDumpType |= MiniDumpWithCodeSegs;
		}

		if (dataseg)
		{
			MiniDumpType |= MiniDumpWithDataSegs;
		}
	}

	LPTOP_LEVEL_EXCEPTION_FILTER WINAPI Exception::SetUnhandledExceptionFilter_Stub(LPTOP_LEVEL_EXCEPTION_FILTER)
	{
		SetFilterHook.uninstall();
		LPTOP_LEVEL_EXCEPTION_FILTER result = ::SetUnhandledExceptionFilter(&ExceptionFilter);
		SetFilterHook.install();
		return result;
	}

	Exception::Exception()
	{
		SetMiniDumpType(Flags::HasFlag("bigminidumps"), Flags::HasFlag("reallybigminidumps"));

		SetFilterHook.initialize(::SetUnhandledExceptionFilter, SetUnhandledExceptionFilter_Stub, HOOK_JUMP);
		SetFilterHook.install();

		::SetUnhandledExceptionFilter(&ExceptionFilter);

		Utils::Hook(0x4B241F, LongJmp_Internal_Stub, HOOK_CALL).install()->quick();
		Utils::Hook(0x61DB44, LongJmp_Internal_Stub, HOOK_CALL).install()->quick();
		Utils::Hook(0x61F17D, LongJmp_Internal_Stub, HOOK_CALL).install()->quick();
		Utils::Hook(0x61F248, LongJmp_Internal_Stub, HOOK_CALL).install()->quick();
		Utils::Hook(0x61F5E7, LongJmp_Internal_Stub, HOOK_CALL).install()->quick();
	}

	Exception::~Exception()
	{
		SetFilterHook.uninstall();
	}
}
