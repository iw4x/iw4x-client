#pragma once

// Version number
#include <version.hpp>

#ifndef RESOURCE_DATA

// Disable irrelevant warnings
#pragma warning(disable: 4100) // Unreferenced parameter (steam has to have them and other stubs as well, due to their calling convention)

#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <wincrypt.h>
#include <time.h>
#include <timeapi.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <WinSock2.h>
#include <Wininet.h>
#include <Urlmon.h>
#include <d3d9.h>

#include <map>
#include <mutex>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <utility>
#include <algorithm>
#include <functional> 
#include <cctype>
#include <locale>
#include <regex>
#include <thread>
#include <chrono>
#include <future>
#include <queue>

// Submodules
// Ignore the warnings, it's no our code!
#pragma warning(push)
#pragma warning(disable: 4005)
#pragma warning(disable: 4389)
#pragma warning(disable: 4702)
#pragma warning(disable: 6001)
#pragma warning(disable: 6011)
#pragma warning(disable: 6031)
#pragma warning(disable: 6255)
#pragma warning(disable: 6258)
#pragma warning(disable: 6386)
#pragma warning(disable: 6387)

#define ZLIB_CONST

#define USE_LTM
#define LTM_DESC
#define LTC_NO_FAST
#define LTC_NO_PROTOTYPES
#define LTC_NO_RSA_BLINDING

#include <zlib.h>
#include <curses.h>
#include <mongoose.h>
#include <json11.hpp>
#include <tomcrypt.h>
#include <wink/signal.hpp>

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

#include <fmt/printf.h>

// Protobuf
#include "proto/network.pb.h"
#include "proto/party.pb.h"
#include "proto/auth.pb.h"
#include "proto/node.pb.h"
#include "proto/rcon.pb.h"

#pragma warning(pop)

#include "Utils\IO.hpp"
#include "Utils\CSV.hpp"
#include "Utils\Utils.hpp"
#include "Utils\WebIO.hpp"
#include "Utils\Memory.hpp"
#include "Utils\String.hpp"
#include "Utils\Hooking.hpp"
#include "Utils\InfoString.hpp"
#include "Utils\Compression.hpp"
#include "Utils\Cryptography.hpp"

#include "Steam\Steam.hpp"

#include "Game\Structs.hpp"
#include "Game\Functions.hpp"

#include "Utils\Stream.hpp"

#include "Components\Loader.hpp"

// Libraries
#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "Crypt32.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "Wininet.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "Urlmon.lib")

// Enable additional literals
using namespace std::literals;

#endif

// Revision number
#define STRINGIZE_(x) #x
#define STRINGIZE(x) STRINGIZE_(x)

#define BASEGAME "iw4x"
#define CLIENT_CONFIG "iw4x_config.cfg"
#define MILESTONE "beta"

#define REVISION_STR STRINGIZE(REVISION)
#if !REVISION_CLEAN
#define REVISION_SUFFIX "*"
#else
#define REVISION_SUFFIX ""
#endif
#define VERSION 4,2,REVISION
#define VERSION_STR "4.2." REVISION_STR

#define Assert_Size(x, size) static_assert(sizeof(x) == size, STRINGIZE(x) " structure has an invalid size.")

// Enable unit-test flag for release builds
//#define FORCE_UNIT_TESTS

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
