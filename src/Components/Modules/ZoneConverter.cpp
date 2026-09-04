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
		constexpr auto* MARKER_FILE = "zone/old/converted.txt";
		constexpr auto* PROGRESS_FILE = "zone/old/converting.txt";
		constexpr auto* LOG_FILE = "zone-conversion.log";

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

		bool RunConverter(const std::string& converter, const std::filesystem::path& zone, const ProgressDialog& dialog)
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
				converter, ZONE_DIRECTORY, zone.generic_string());

			PROCESS_INFORMATION process{};
			const auto started = CreateProcessA(converter.data(), commandLine.data(), nullptr, nullptr,
				log != INVALID_HANDLE_VALUE, CREATE_NO_WINDOW, nullptr, nullptr, &startup, &process);

			const auto error = GetLastError();

			if (log != INVALID_HANDLE_VALUE)
			{
				CloseHandle(log);
			}

			if (!started)
			{
				Log(std::format("Could not start \"{}\" ({})", converter, error));
				return false;
			}

			CloseHandle(process.hThread);

			while (MsgWaitForMultipleObjects(1, &process.hProcess, FALSE, INFINITE, QS_ALLINPUT) == WAIT_OBJECT_0 + 1)
			{
				dialog.Pump();
			}

			DWORD exitCode = 1;
			GetExitCodeProcess(process.hProcess, &exitCode);
			CloseHandle(process.hProcess);

			return exitCode == 0;
		}

		bool CopyAcross(const std::filesystem::path& file)
		{
			std::error_code error;
			std::filesystem::copy_file(file,
				std::filesystem::path(ZONE_DIRECTORY) / file.filename(),
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

		void Run()
		{
			if (Utils::IO::FileExists(MARKER_FILE))
			{
				return;
			}

			const auto resuming = Utils::IO::FileExists(PROGRESS_FILE);
			const std::filesystem::path source = resuming ? BACKUP_DIRECTORY : ZONE_DIRECTORY;

			if (!Utils::IO::DirectoryExists(source))
			{
				return;
			}

			std::vector<std::filesystem::path> toConvert;
			std::vector<std::filesystem::path> toCopy;

			for (const auto& entry : Utils::IO::ListFiles(source, false))
			{
				if (!entry.is_regular_file())
				{
					continue;
				}

				if (ProbeWordSize(entry.path()) == WordSize::X64)
				{
					toConvert.push_back(entry.path());
				}
				else
				{
					toCopy.push_back(entry.path());
				}
			}

			if (toConvert.empty())
			{
				if (resuming)
				{
					Utils::IO::WriteFile(MARKER_FILE, "");
					Utils::IO::RemoveFile(PROGRESS_FILE);
				}

				return;
			}

			const auto converter = FindConverter();
			if (!converter)
			{
				Fail(std::format("This installation ships the Microsoft Store's x64 fastfiles, which have to be "
					"converted before the game can read them, and {} was not found next to the game.",
					CONVERTER_NAMES[0]));
				return;
			}

			if (!resuming)
			{
				std::error_code error;
				std::filesystem::rename(ZONE_DIRECTORY, BACKUP_DIRECTORY, error);

				if (error)
				{
					Fail(std::format("Could not move \"{}\" to \"{}\": {}",
						ZONE_DIRECTORY, BACKUP_DIRECTORY, error.message()));
					return;
				}

				for (auto* list : {&toConvert, &toCopy})
				{
					for (auto& path : *list)
					{
						path = std::filesystem::path(BACKUP_DIRECTORY) / path.filename();
					}
				}
			}

			Utils::IO::CreateDir(ZONE_DIRECTORY);
			Utils::IO::WriteFile(PROGRESS_FILE, "");

			Log(std::format("Converting {} x64 fastfiles to x86", toConvert.size()));

			ProgressDialog dialog("IW4x - Converting fastfiles", static_cast<int>(toConvert.size() + toCopy.size()));

			for (const auto& path : toCopy)
			{
				dialog.Begin(std::format("Copying {}", path.filename().generic_string()));

				if (!CopyAcross(path))
				{
					Fail(std::format("Could not copy \"{}\" into \"{}\".", path.generic_string(), ZONE_DIRECTORY));
					return;
				}

				dialog.Finish();
			}

			for (const auto& path : toConvert)
			{
				dialog.Begin(std::format("Converting {}", path.filename().generic_string()));

				if (!RunConverter(*converter, path, dialog))
				{
					Fail(std::format("Converting \"{}\" failed. See \"{}\" for what the converter said. "
						"The originals are still in \"{}\".",
						path.generic_string(), LOG_FILE, BACKUP_DIRECTORY));
					return;
				}

				dialog.Finish();
			}

			Utils::IO::WriteFile(MARKER_FILE, std::format("Converted {} fastfiles from the x64 layout.\n", toConvert.size()));
			Utils::IO::RemoveFile(PROGRESS_FILE);

			Log("Finished converting fastfiles");
		}
	}

	ZoneConverter::ZoneConverter()
	{
		Run();
	}
}
