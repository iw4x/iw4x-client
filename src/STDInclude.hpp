#pragma once

#ifndef RC_INVOKED

#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _USE_MATH_DEFINES

#include <Windows.h>
#include <WinSock2.h>
#include <ShlObj.h>
#include <timeapi.h>
#include <shellapi.h>
#include <wininet.h>
#include <d3d9.h>
#include <AclAPI.h>
#include <Psapi.h>
#include <TlHelp32.h>
#include <Shlwapi.h>

#include <DbgHelp.h>

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cinttypes>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <format>
#include <fstream>
#include <future>
#include <limits>
#include <optional>
#include <queue>
#include <random>
#include <ranges>
#include <regex>
#include <source_location>
#include <sstream>
#include <thread>
#include <type_traits>
#include <map>
#include <set>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>

#include <d3dx9tex.h>
#pragma comment(lib, "D3dx9.lib")

#include <XInput.h>
#pragma comment (lib, "xinput.lib")

#include <dwmapi.h>
#pragma comment (lib, "dwmapi.lib")

// Ignore the warnings
#pragma warning(push)
#pragma warning(disable: 4100)
#pragma warning(disable: 26812)

// Enable additional literals
using namespace std::literals;

#ifdef max
	#undef max
#endif

#ifdef min
	#undef min
#endif

#ifdef GetObject
	#undef GetObject
#endif

#define AssertSize(x, size) \
	static_assert(sizeof(x) == (size), \
		"Structure has an invalid size. " #x " must be " #size " bytes")

#define AssertOffset(x, y, offset) \
	static_assert(offsetof(x, y) == (offset), \
		#x "::" #y " is not at the right offset. Must be at " #offset)

#define AssertIn(x, y) assert(static_cast<unsigned int>(x) < static_cast<unsigned int>(y))

#define AssertUnreachable assert(0 && "unreachable")

#include <gsl/gsl>
#include <json.hpp>
#include <tomcrypt.h>

#pragma warning(pop)

#include "Utils/Memory.hpp" // Breaks order on purpose

#include "Utils/Cache.hpp"
#include "Utils/Chain.hpp"
#include "Utils/Concurrency.hpp"
#include "Utils/Cryptography.hpp"
#include "Utils/CSV.hpp"
#include "Utils/Entities.hpp"
#include "Utils/Hooking.hpp"
#include "Utils/IO.hpp"
#include "Utils/Library.hpp"
#include "Utils/Maths.hpp"
#include "Utils/NamedMutex.hpp"
#include "Utils/String.hpp"
#include "Utils/Thread.hpp"
#include "Utils/Time.hpp"
#include "Utils/Utils.hpp"

#include "Steam/Steam.hpp" // Some definitions are used in functions and structs

#include "Game/Structs.hpp"
#include "Game/Game.hpp"

#include <Game/Scripting/Function.hpp>
#include <Game/Scripting/StackIsolation.hpp>

#include "Utils/Stream.hpp" // Breaks order on purpose

#include "Components/Loader.hpp"

// Libraries
#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "Crypt32.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "Wininet.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "Urlmon.lib")
#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib, "rpcrt4.lib")
#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "ntdll.lib")

#endif

#define BASEGAME "iw4x"
#define BASEGAME_NAME "iw4mp_ceg.exe"
#define CLIENT_CONFIG "iw4x_config.cfg"

// Resource stuff
#ifdef APSTUDIO_INVOKED
#ifndef APSTUDIO_READONLY_SYMBOLS
// Defines below make accessing the resources from the code easier.
#define _APS_NEXT_RESOURCE_VALUE        102
#define _APS_NEXT_COMMAND_VALUE         40001
#define _APS_NEXT_CONTROL_VALUE         1001
#define _APS_NEXT_SYMED_VALUE           101
#endif
#endif
