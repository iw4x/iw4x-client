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
		constexpr auto* ZONE_DIRECTORY = "zone/english";
		constexpr auto* BACKUP_DIRECTORY = "zone/old";
		constexpr auto* DLC_DIRECTORY = "zone/dlc";
		constexpr auto* DLC_BACKUP_DIRECTORY = "zone/dlc_old";
		constexpr auto* MARKER_NAME = "converted.txt";
		constexpr auto* PROGRESS_NAME = "converting.txt";
		constexpr auto* LOG_FILE = "zone-conversion.log";

		struct Location
		{
			const char* zone;
			const char* backup;
		};

		constexpr Location LOCATIONS[] = {
			{ZONE_DIRECTORY, BACKUP_DIRECTORY},
			{DLC_DIRECTORY, DLC_BACKUP_DIRECTORY},
		};

		std::string MarkerFile(const Location& where)
		{
			return std::format("{}/{}", where.backup, MARKER_NAME);
		}

		std::string ProgressFile(const Location& where)
		{
			return std::format("{}/{}", where.backup, PROGRESS_NAME);
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
			const char* output, const ProgressDialog& dialog)
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

		bool CopyAcross(const std::filesystem::path& file, const char* destination)
		{
			std::error_code error;
			std::filesystem::copy_file(file,
				std::filesystem::path(destination) / file.filename(),
				std::filesystem::copy_options::overwrite_existing,
				error);

			if (error)
			{
				Log(std::format("Could not copy \"{}\": {}", file.generic_string(), error.message()));
				return false;
			}

			return true;
		}

		void Fail(const std::string& message)
		{
			Log(message);
			MessageBoxA(nullptr, message.data(), "IW4x", MB_ICONERROR | MB_OK);
		}

		struct Plan
		{
			const Location* where = nullptr;
			bool resuming = false;
			std::vector<std::filesystem::path> toConvert;
			std::vector<std::filesystem::path> toCopy;
		};

		std::optional<Plan> Survey(const Location& where)
		{
			if (Utils::IO::FileExists(MarkerFile(where)))
			{
				return {};
			}

			const auto resuming = Utils::IO::FileExists(ProgressFile(where));
			const std::filesystem::path source = resuming ? where.backup : where.zone;

			if (!Utils::IO::DirectoryExists(source))
			{
				return {};
			}

			Plan plan;
			plan.where = &where;
			plan.resuming = resuming;

			for (const auto& entry : Utils::IO::ListFiles(source, false))
			{
				if (!entry.is_regular_file())
				{
					continue;
				}

				if (ProbeWordSize(entry.path()) == WordSize::X64)
				{
					plan.toConvert.push_back(entry.path());
				}
				else
				{
					plan.toCopy.push_back(entry.path());
				}
			}

			if (plan.toConvert.empty())
			{
				if (resuming)
				{
					Utils::IO::WriteFile(MarkerFile(where), "");
					Utils::IO::RemoveFile(ProgressFile(where));
				}

				return {};
			}

			return plan;
		}

		bool Prepare(Plan& plan)
		{
			const auto& where = *plan.where;

			if (!plan.resuming)
			{
				std::error_code error;
				std::filesystem::rename(where.zone, where.backup, error);

				if (error)
				{
					Fail(std::format("Could not move \"{}\" to \"{}\": {}",
						where.zone, where.backup, error.message()));
					return false;
				}

				for (auto* list : {&plan.toConvert, &plan.toCopy})
				{
					for (auto& path : *list)
					{
						path = std::filesystem::path(where.backup) / path.filename();
					}
				}
			}

			Utils::IO::CreateDir(where.zone);
			Utils::IO::WriteFile(ProgressFile(where), "");

			return true;
		}

		bool Process(const Plan& plan, const std::string& converter, ProgressDialog& dialog)
		{
			const auto& where = *plan.where;

			for (const auto& path : plan.toCopy)
			{
				dialog.Begin(std::format("Copying {}", path.filename().generic_string()));

				if (!CopyAcross(path, where.zone))
				{
					Fail(std::format("Could not copy \"{}\" into \"{}\".", path.generic_string(), where.zone));
					return false;
				}

				dialog.Finish();
			}

			for (const auto& path : plan.toConvert)
			{
				dialog.Begin(std::format("Converting {}", path.filename().generic_string()));

				const auto outcome = RunConverter(converter, path, where.zone, dialog);

				if (!outcome.started)
				{
					Fail(std::format("\"{}\" could not be started: {}.\n\n"
						"Antivirus software such as Windows Defender often blocks or quarantines it. "
						"Add an exclusion for the IW4x folder and start the game again.\n\n"
						"No fastfile was converted with it; the originals are still in \"{}\".",
						converter, outcome.startError, where.backup));
					return false;
				}

				if (outcome.exitCode != 0)
				{
					Fail(std::format("Converting \"{}\" failed (\"{}\" exited with code {}). See \"{}\" for "
						"what the converter said. The originals are still in \"{}\".",
						path.generic_string(), converter, outcome.exitCode, LOG_FILE, where.backup));
					return false;
				}

				dialog.Finish();
			}

			Utils::IO::WriteFile(MarkerFile(where),
				std::format("Converted {} fastfiles from the x64 layout.\n", plan.toConvert.size()));
			Utils::IO::RemoveFile(ProgressFile(where));

			return true;
		}

		void Run()
		{
			std::vector<Plan> plans;

			for (const auto& where : LOCATIONS)
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
				Fail(std::format("This installation ships the Microsoft Store's x64 fastfiles, which have to be "
					"converted before the game can read them, and {} was not found next to the game.\n\n"
					"If you did install it, antivirus software such as Windows Defender may have quarantined it. "
					"Add an exclusion for the IW4x folder and start the game again.",
					CONVERTER_NAMES[0]));
				return;
			}

			std::size_t steps = 0;
			std::size_t conversions = 0;

			for (const auto& plan : plans)
			{
				steps += plan.toConvert.size() + plan.toCopy.size();
				conversions += plan.toConvert.size();
			}

			Log(std::format("Converting {} x64 fastfiles to x86", conversions));

			ProgressDialog dialog("IW4x - Converting fastfiles", static_cast<int>(steps));

			for (auto& plan : plans)
			{
				if (!Prepare(plan) || !Process(plan, *converter, dialog))
				{
					return;
				}
			}

			Log("Finished converting fastfiles");
		}
	}

	ZoneConverter::ZoneConverter()
	{
		Run();
	}
}
