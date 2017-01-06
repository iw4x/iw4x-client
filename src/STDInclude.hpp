#pragma once

// Version number
#include "version.h"

#ifndef RC_INVOKED

#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
//#include <stdio.h>
//#include <string.h>
//#include <wincrypt.h>
//#include <time.h>
#include <timeapi.h>
#include <shellapi.h>
//#include <shlwapi.h>
//#include <WinSock2.h>
#include <Wininet.h>
//#include <Urlmon.h>
#include <d3d9.h>
#include <Aclapi.h>

//#include <map>
//#include <mutex>
//#include <string>
//#include <vector>
#include <sstream>
#include <fstream>
//#include <utility>
//#include <algorithm>
//#include <functional>
#include <cctype>
//#include <locale>
#include <regex>
#include <thread>
//#include <chrono>
#include <future>
#include <queue>
#include <unordered_map>

// Experimental C++17 features
#include <filesystem>

#ifdef ENABLE_DXSDK
#include <D3dx9tex.h>
#pragma comment(lib, "D3dx9.lib")
#endif

// Usefull for debugging
template <size_t S> class Sizer { };
#define BindNum(x, y) Sizer<x> y;
#define SizeOf(x, y) BindNum(sizeof(x), y)
#define OffsetOf(x, y, z) BindNum(offsetof(x, y), z)

// Submodules
// Ignore the warnings, it's no our code!
#pragma warning(push)
#pragma warning(disable: 4005)
#pragma warning(disable: 4100)
#pragma warning(disable: 4389)
#pragma warning(disable: 4702)
#pragma warning(disable: 4996) // _CRT_SECURE_NO_WARNINGS
#pragma warning(disable: 6001)
#pragma warning(disable: 6011)
#pragma warning(disable: 6031)
#pragma warning(disable: 6255)
#pragma warning(disable: 6258)
#pragma warning(disable: 6386)
#pragma warning(disable: 6387)

#include <zlib.h>
#include <curses.h>
#include <mongoose.h>
#include <json11.hpp>
#include <tomcrypt.h>
#ifndef DISABLE_BITMESSAGE
#include <BitMRC.h>
#endif

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

// Protobuf
#include "proto/network.pb.h"
#include "proto/party.pb.h"
#include "proto/auth.pb.h"
#include "proto/node.pb.h"
#include "proto/rcon.pb.h"

#pragma warning(pop)

#include "Utils\IO.hpp"
#include "Utils\CSV.hpp"
#include "Utils\Time.hpp"
#include "Utils\Cache.hpp"
#include "Utils\Chain.hpp"
#include "Utils\Utils.hpp"
#include "Utils\WebIO.hpp"
#include "Utils\Memory.hpp"
#include "Utils\String.hpp"
#include "Utils\Hooking.hpp"
#include "Utils\Library.hpp"
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
#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib, "rpcrt4.lib")

// Enable additional literals
using namespace std::literals;

#endif

#define STRINGIZE_(x) #x
#define STRINGIZE(x) STRINGIZE_(x)

#define BASEGAME "iw4x"
#define CLIENT_CONFIG "iw4x_config.cfg"

#define AssertSize(x, size) static_assert(sizeof(x) == size, STRINGIZE(x) " structure has an invalid size.")
#define AssertOffset(x, y, offset) static_assert(offsetof(x, y) == offset, STRINGIZE(x) "::" STRINGIZE(y) " is not at the right offset.")

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
