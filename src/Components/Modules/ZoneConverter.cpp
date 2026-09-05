#include <zlib.h>

#include "ZoneConverter.hpp"

#pragma comment(linker, "/manifestdependency:\"type='win32' "                \
	"name='Microsoft.Windows.Common-Controls' version='6.0.0.0' "            \
	"processorArchitecture='*' publicKeyToken='6595b64144ccf1df' "           \
	"language='*'\"")

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

namespace Components
{
	namespace
	{
		constexpr auto* ZONE_ROOT = "zone";
		constexpr auto* MARKER_NAME = "converted.txt";
		constexpr auto* LOG_FILE = "zone-conversion.log";

		struct Location
		{
			std::string zone;
			std::string backup;
		};

		bool IsBackupDirectory(const std::string& name)
		{
			return name == "old" || name.ends_with("_old");
		}

		std::string BackupFor(const std::string& name)
		{
			if (name == "english")
			{
				return std::format("{}/old", ZONE_ROOT);
			}

			return std::format("{}/{}_old", ZONE_ROOT, name);
		}

		std::vector<Location> Locations()
		{
			std::vector<Location> locations;

			if (!Utils::IO::DirectoryExists(ZONE_ROOT))
			{
				return locations;
			}

			for (const auto& entry : Utils::IO::ListFiles(ZONE_ROOT, false))
			{
				if (!entry.is_directory())
				{
					continue;
				}

				const auto name = entry.path().filename().generic_string();

				if (IsBackupDirectory(name))
				{
					continue;
				}

				locations.push_back({entry.path().generic_string(), BackupFor(name)});
			}

			return locations;
		}

		std::string MarkerFile(const Location& where)
		{
			return std::format("{}/{}", where.backup, MARKER_NAME);
		}

		const char* const CONVERTER_NAMES[] = {
			"Unlinker.exe",
			"tools/Unlinker.exe",
		};

		constexpr std::size_t PREAMBLE_SIZE = 8 + 4 + 1 + 8;
		constexpr std::size_t AUTHED_CHUNK_SIZE = 0x2000;
		constexpr std::size_t SIZE_HEADER_SIZE = 40;
		constexpr std::size_t ASSET_LIST_X64_SIZE = 32;
		constexpr int ZONE_VERSION_PC = 276;

		enum class WordSize
		{
			X86,
			X64,
		};

		void Log(const std::string& message)
		{
			OutputDebugStringA(message.data());
			OutputDebugStringA("\n");

			std::ofstream log(LOG_FILE, std::ios::app);
			if (log.is_open())
			{
				log << message << "\n";
			}
		}

		std::optional<std::size_t> PayloadOffset(const std::uint8_t* header)
		{
			int version;
			std::memcpy(&version, header + 8, sizeof(version));

			if (version != ZONE_VERSION_PC)
			{
				return {};
			}

			if (std::memcmp(header, "IWffu100", 8) == 0)
			{
				return PREAMBLE_SIZE;
			}

			if (std::memcmp(header, "IWff0100", 8) == 0 || std::memcmp(header, "ABff0100", 8) == 0)
			{
				return PREAMBLE_SIZE + AUTHED_CHUNK_SIZE + AUTHED_CHUNK_SIZE;
			}

			return {};
		}

		std::optional<std::string> InflatePrefix(std::ifstream& file, const std::size_t offset, const std::size_t wanted)
		{
			constexpr std::size_t INPUT_LIMIT = 0x10000;

			file.clear();
			file.seekg(static_cast<std::streamoff>(offset));
			if (!file)
			{
				return {};
			}

			std::string input(INPUT_LIMIT, '\0');
			file.read(input.data(), static_cast<std::streamsize>(input.size()));
			input.resize(static_cast<std::size_t>(file.gcount()));

			if (input.empty())
			{
				return {};
			}

			z_stream stream{};
			if (inflateInit(&stream) != Z_OK)
			{
				return {};
			}

			std::string output(wanted, '\0');
			stream.next_in = reinterpret_cast<Bytef*>(input.data());
			stream.avail_in = static_cast<uInt>(input.size());
			stream.next_out = reinterpret_cast<Bytef*>(output.data());
			stream.avail_out = static_cast<uInt>(output.size());

			const auto result = inflate(&stream, Z_NO_FLUSH);
			const auto produced = output.size() - stream.avail_out;
			inflateEnd(&stream);

			if (result != Z_OK && result != Z_STREAM_END)
			{
				return {};
			}

			output.resize(produced);
			return output;
		}

		bool AllZero(const std::uint8_t* bytes, const std::size_t size)
		{
			return std::all_of(bytes,
			                   bytes + size,
			                   [](const std::uint8_t byte)
			                   {
				                   return byte == 0;
			                   });
		}

		std::uint32_t ReadCount(const std::uint8_t* bytes)
		{
			std::uint32_t value;
			std::memcpy(&value, bytes, sizeof(value));

			return value;
		}

		bool StreamPointer(const std::uint32_t count, const std::uint8_t* bytes)
		{
			std::uint64_t value;
			std::memcpy(&value, bytes, sizeof(value));

			constexpr auto following = ~std::uint64_t(0);

			if (count == 0)
			{
				return value == 0;
			}

			return value == following || value == following - 1;
		}

		std::optional<WordSize> ProbeWordSize(const std::filesystem::path& path)
		{
			constexpr auto WANTED = SIZE_HEADER_SIZE + ASSET_LIST_X64_SIZE;

			std::ifstream file(path, std::ios::binary);
			if (!file.is_open())
			{
				return {};
			}

			std::uint8_t header[PREAMBLE_SIZE]{};
			file.read(reinterpret_cast<char*>(header), sizeof(header));
			if (file.gcount() != static_cast<std::streamsize>(sizeof(header)))
			{
				return {};
			}

			const auto offset = PayloadOffset(header);
			if (!offset)
			{
				return {};
			}

			const auto payload = InflatePrefix(file, *offset, WANTED);
			if (!payload || payload->size() < WANTED)
			{
				return {};
			}

			const auto* list = reinterpret_cast<const std::uint8_t*>(payload->data()) + SIZE_HEADER_SIZE;

			const auto x64 = AllZero(list + 4, 4)
				&& StreamPointer(ReadCount(list), list + 8)
				&& AllZero(list + 20, 4)
				&& StreamPointer(ReadCount(list + 16), list + 24);

			return x64 ? WordSize::X64 : WordSize::X86;
		}

		std::optional<std::string> FindConverter()
		{
			for (const auto* name : CONVERTER_NAMES)
			{
				if (Utils::IO::FileExists(name))
				{
					return name;
				}
			}

			return {};
		}

		LRESULT CALLBACK ProgressWindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
		{
			switch (message)
			{
			case WM_CTLCOLORSTATIC:
				SetBkMode(reinterpret_cast<HDC>(wParam), TRANSPARENT);
				return reinterpret_cast<LRESULT>(GetSysColorBrush(COLOR_BTNFACE));

			case WM_CLOSE:
				return 0;

			default:
				return DefWindowProcA(window, message, wParam, lParam);
			}
		}

		class VisualStyles
		{
		public:
			VisualStyles()
			{
				ACTCTXA context{};
				context.cbSize = sizeof(context);
				context.dwFlags = ACTCTX_FLAG_RESOURCE_NAME_VALID | ACTCTX_FLAG_HMODULE_VALID;
				context.lpResourceName = ISOLATIONAWARE_MANIFEST_RESOURCE_ID;
				context.hModule = reinterpret_cast<HMODULE>(&__ImageBase);

				this->context_ = CreateActCtxA(&context);

				if (this->context_ != INVALID_HANDLE_VALUE)
				{
					this->active_ = ActivateActCtx(this->context_, &this->cookie_) != FALSE;
				}
			}

			~VisualStyles()
			{
				if (this->active_)
				{
					DeactivateActCtx(0, this->cookie_);
				}

				if (this->context_ != INVALID_HANDLE_VALUE)
				{
					ReleaseActCtx(this->context_);
				}
			}

			VisualStyles(const VisualStyles&) = delete;
			VisualStyles& operator=(const VisualStyles&) = delete;

		private:
			HANDLE context_ = INVALID_HANDLE_VALUE;
			ULONG_PTR cookie_ = 0;
			bool active_ = false;
		};

		class ProgressDialog
		{
		public:
			ProgressDialog(const std::string& title, const int steps)
				: steps_(steps)
			{
				INITCOMMONCONTROLSEX controls{};
				controls.dwSize = sizeof(controls);
				controls.dwICC = ICC_PROGRESS_CLASS;
				InitCommonControlsEx(&controls);

				WNDCLASSEXA windowClass{};
				windowClass.cbSize = sizeof(windowClass);
				windowClass.lpfnWndProc = ProgressWindowProc;
				windowClass.hInstance = GetModuleHandleA(nullptr);
				windowClass.hCursor = LoadCursorA(nullptr, IDC_ARROW);
				windowClass.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
				windowClass.lpszClassName = "IW4xZoneConverter";
				RegisterClassExA(&windowClass);

				constexpr auto width = 460;
				constexpr auto height = 150;

				const auto x = (GetSystemMetrics(SM_CXSCREEN) - width) / 2;
				const auto y = (GetSystemMetrics(SM_CYSCREEN) - height) / 2;

				this->window_ = CreateWindowExA(WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
					windowClass.lpszClassName,
					title.data(),
					WS_POPUP | WS_CAPTION | WS_VISIBLE,
					x, y, width, height,
					nullptr, nullptr, windowClass.hInstance, nullptr);

				if (!this->window_)
				{
					return;
				}

				RECT client{};
				GetClientRect(this->window_, &client);
				const auto inner = client.right - 32;

				this->label_ = CreateWindowExA(0, "STATIC", "", WS_CHILD | WS_VISIBLE | SS_PATHELLIPSIS,
					16, 18, inner, 18, this->window_, nullptr, windowClass.hInstance, nullptr);

				this->bar_ = CreateWindowExA(0, PROGRESS_CLASSA, "", WS_CHILD | WS_VISIBLE,
					16, 46, inner, 20, this->window_, nullptr, windowClass.hInstance, nullptr);

				this->count_ = CreateWindowExA(0, "STATIC", "", WS_CHILD | WS_VISIBLE,
					16, 78, inner, 18, this->window_, nullptr, windowClass.hInstance, nullptr);

				this->font_ = CreateMessageFont();
				for (auto* control : {this->label_, this->bar_, this->count_})
				{
					SendMessageA(control, WM_SETFONT, reinterpret_cast<WPARAM>(this->font_), TRUE);
				}

				SendMessageA(this->bar_, PBM_SETRANGE32, 0, steps);
				SendMessageA(this->bar_, PBM_SETPOS, 0, 0);

				SetForegroundWindow(this->window_);
				this->Pump();
			}

			~ProgressDialog()
			{
				if (this->window_)
				{
					DestroyWindow(this->window_);
					this->window_ = nullptr;
				}

				this->Pump();

				if (this->font_)
				{
					DeleteObject(this->font_);
					this->font_ = nullptr;
				}
			}

			ProgressDialog(const ProgressDialog&) = delete;
			ProgressDialog& operator=(const ProgressDialog&) = delete;

			void Begin(const std::string& what)
			{
				if (!this->window_)
				{
					return;
				}

				SetWindowTextA(this->label_, what.data());
				SetWindowTextA(this->count_, std::format("{} of {}", this->step_ + 1, this->steps_).data());
				this->Pump();
			}

			void Finish()
			{
				++this->step_;

				if (this->window_)
				{
					SendMessageA(this->bar_, PBM_SETPOS, this->step_, 0);
					this->Pump();
				}
			}

			void Pump() const
			{
				MSG message;
				while (PeekMessageA(&message, nullptr, 0, 0, PM_REMOVE))
				{
					TranslateMessage(&message);
					DispatchMessageA(&message);
				}
			}

		private:
			static HFONT CreateMessageFont()
			{
				NONCLIENTMETRICSA metrics{};
				metrics.cbSize = sizeof(metrics);

				if (!SystemParametersInfoA(SPI_GETNONCLIENTMETRICS, sizeof(metrics), &metrics, 0))
				{
					return nullptr;
				}

				return CreateFontIndirectA(&metrics.lfMessageFont);
			}

			VisualStyles styles_;
			HWND window_ = nullptr;
			HWND label_ = nullptr;
			HWND bar_ = nullptr;
			HWND count_ = nullptr;
			HFONT font_ = nullptr;
			int step_ = 0;
			int steps_ = 0;
		};

		struct ConverterOutcome
		{
			bool started = false;
			std::string startError;
			DWORD exitCode = 0;
		};

		ConverterOutcome RunConverter(const std::string& converter, const std::filesystem::path& zone,
			const std::string& output, const ProgressDialog& dialog)
		{
			SECURITY_ATTRIBUTES inheritable{};
			inheritable.nLength = sizeof(inheritable);
			inheritable.bInheritHandle = TRUE;

			auto* log = CreateFileA(LOG_FILE, FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE,
				&inheritable, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

			STARTUPINFOA startup{};
			startup.cb = sizeof(startup);
			startup.dwFlags = STARTF_USESHOWWINDOW;
			startup.wShowWindow = SW_HIDE;

			if (log != INVALID_HANDLE_VALUE)
			{
				startup.dwFlags |= STARTF_USESTDHANDLES;
				startup.hStdOutput = log;
				startup.hStdError = log;
			}

			auto commandLine = std::format("\"{}\" --game IW4MS --convert-to IW4 -o \"{}\" \"{}\"",
				converter, output, zone.generic_string());

			PROCESS_INFORMATION process{};
			const auto started = CreateProcessA(converter.data(), commandLine.data(), nullptr, nullptr,
				log != INVALID_HANDLE_VALUE, CREATE_NO_WINDOW, nullptr, nullptr, &startup, &process);

			ConverterOutcome outcome;

			if (!started)
			{
				const auto error = GetLastError();
				auto reason = Utils::GetLastWindowsError();
				Utils::String::Trim(reason);

				while (!reason.empty() && reason.back() == '.')
				{
					reason.pop_back();
				}

				outcome.startError = std::format("{} ({})", reason.empty() ? "unknown error" : reason, error);
			}

			if (log != INVALID_HANDLE_VALUE)
			{
				CloseHandle(log);
			}

			if (!started)
			{
				Log(std::format("Could not start \"{}\": {}", converter, outcome.startError));
				return outcome;
			}

			CloseHandle(process.hThread);

			while (MsgWaitForMultipleObjects(1, &process.hProcess, FALSE, INFINITE, QS_ALLINPUT) == WAIT_OBJECT_0 + 1)
			{
				dialog.Pump();
			}

			outcome.started = true;
			outcome.exitCode = 1;
			GetExitCodeProcess(process.hProcess, &outcome.exitCode);
			CloseHandle(process.hProcess);

			return outcome;
		}

		[[noreturn]] void Fail(const std::string& message)
		{
			Log(message);
			MessageBoxA(nullptr, message.data(), "IW4x", MB_ICONERROR | MB_OK);

			ExitProcess(1);
		}

		struct Job
		{
			std::string name;
			bool moved = false;
		};

		struct Plan
		{
			Location where;
			std::vector<Job> jobs;
		};

		std::optional<Plan> Survey(const Location& where)
		{
			Plan plan;
			plan.where = where;

			for (const auto& entry : Utils::IO::ListFiles(where.zone, false))
			{
				if (!entry.is_regular_file())
				{
					continue;
				}

				if (ProbeWordSize(entry.path()) == WordSize::X64)
				{
					plan.jobs.push_back({entry.path().filename().generic_string(), false});
				}
			}

			if (Utils::IO::DirectoryExists(where.backup))
			{
				for (const auto& entry : Utils::IO::ListFiles(where.backup, false))
				{
					if (!entry.is_regular_file())
					{
						continue;
					}

					auto name = entry.path().filename().generic_string();

					if (Utils::IO::FileExists(std::format("{}/{}", where.zone, name)))
					{
						continue;
					}

					if (ProbeWordSize(entry.path()) == WordSize::X64)
					{
						plan.jobs.push_back({std::move(name), true});
					}
				}
			}

			if (plan.jobs.empty())
			{
				return {};
			}

			return plan;
		}

		void MoveAside(const Location& where, const std::string& name)
		{
			const std::filesystem::path source = std::format("{}/{}", where.zone, name);
			const std::filesystem::path destination = std::format("{}/{}", where.backup, name);

			std::error_code stale;
			std::filesystem::remove(destination, stale);

			std::error_code error;
			std::filesystem::rename(source, destination, error);

			if (error)
			{
				Fail(std::format("Could not move \"{}\" to \"{}\": {}\n\n"
					"The game cannot read that fastfile in the layout it is in.",
					source.generic_string(), destination.generic_string(), error.message()));
			}
		}

		std::string Convert(const Location& where, const Job& job, const std::string& converter,
			ProgressDialog& dialog)
		{
			const std::filesystem::path source = std::format("{}/{}", where.backup, job.name);
			const auto outcome = RunConverter(converter, source, where.zone, dialog);

			if (!outcome.started)
			{
				Fail(std::format("\"{}\" could not be started: {}.\n\n"
					"Antivirus software such as Windows Defender often blocks or quarantines it. "
					"Add an exclusion for the IW4x folder and start the game again.\n\n"
					"The originals are in \"{}\".",
					converter, outcome.startError, where.backup));
			}

			if (outcome.exitCode != 0)
			{
				return std::format("\"{}\" exited with code {}", converter, outcome.exitCode);
			}

			const auto converted = std::format("{}/{}", where.zone, job.name);

			if (!Utils::IO::FileExists(converted))
			{
				return std::format("\"{}\" wrote nothing to \"{}\"", converter, converted);
			}

			if (ProbeWordSize(converted) == WordSize::X64)
			{
				return std::format("\"{}\" is still in the x64 layout", converted);
			}

			return {};
		}

		void Process(const Plan& plan, const std::string& converter, ProgressDialog& dialog,
			std::vector<std::string>& failures)
		{
			const auto& where = plan.where;
			const auto before = failures.size();

			Utils::IO::CreateDir(where.backup);

			for (const auto& job : plan.jobs)
			{
				dialog.Begin(std::format("Converting {}", job.name));

				if (!job.moved)
				{
					MoveAside(where, job.name);
				}

				if (const auto reason = Convert(where, job, converter, dialog); !reason.empty())
				{
					std::error_code partial;
					std::filesystem::remove(std::format("{}/{}", where.zone, job.name), partial);

					Log(std::format("Could not convert \"{}/{}\": {}", where.backup, job.name, reason));
					failures.push_back(job.name);
				}

				dialog.Finish();
			}

			Utils::IO::WriteFile(MarkerFile(where),
				std::format("Converted {} of {} fastfiles from the x64 layout.\n",
					plan.jobs.size() - (failures.size() - before), plan.jobs.size()));
		}

		void Run()
		{
			std::vector<Plan> plans;

			for (const auto& where : Locations())
			{
				if (auto plan = Survey(where))
				{
					plans.push_back(std::move(*plan));
				}
			}

			if (plans.empty())
			{
				return;
			}

			const auto converter = FindConverter();
			if (!converter)
			{
				Fail(std::format("This installation ships x64 fastfiles, which have to be "
					"converted before the game can read them, and {} was not found next to the game.\n\n"
					"If you did install it, antivirus software such as Windows Defender may have quarantined it. "
					"Add an exclusion for the IW4x folder and start the game again.",
					CONVERTER_NAMES[0]));
			}

			std::size_t steps = 0;

			for (const auto& plan : plans)
			{
				steps += plan.jobs.size();
			}

			Log(std::format("Converting {} x64 fastfiles to x86", steps));

			std::vector<std::string> failures;

			{
				ProgressDialog dialog("IW4x - Converting fastfiles", static_cast<int>(steps));

				for (const auto& plan : plans)
				{
					Process(plan, *converter, dialog, failures);
				}
			}

			Log(std::format("Finished converting fastfiles with {} failures", failures.size()));

			if (!failures.empty())
			{
				std::string names;

				for (const auto& failure : failures)
				{
					names.append(failure).append("\n");
				}

				MessageBoxA(nullptr,
					std::format("{} of {} fastfiles could not be converted:\n\n{}\n"
						"See \"{}\" for what the converter said. The game will start without them.",
						failures.size(), steps, names, LOG_FILE).data(),
					"IW4x", MB_ICONWARNING | MB_OK);
			}
		}
	}

	ZoneConverter::ZoneConverter()
	{
		Run();
	}
}
