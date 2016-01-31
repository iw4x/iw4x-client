#pragma once

// Disable irrelevant warnings
#pragma warning(disable: 4100) // Unreferenced parameter (steam has to have them and other stubs as well, due to their calling convention)

#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <wincrypt.h>
#include <time.h>
#include <timeapi.h>
#include <shellapi.h>
#include <WinSock2.h>
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

// Submodules
// Ignore the warnings, it's no our code!
#pragma warning(push)
#pragma warning(disable: 4005)
#pragma warning(disable: 6001)
#pragma warning(disable: 6011)
#pragma warning(disable: 6031)
#pragma warning(disable: 6255)
#pragma warning(disable: 6258)
#pragma warning(disable: 6386)
#pragma warning(disable: 6387)

#define ZLIB_CONST

#define TFM_DESC
#define LTC_NO_FAST
#define LTC_NO_PROTOTYPES
#define LTC_NO_RSA_BLINDING

#define ASIO_STANDALONE

#include <zlib.h>
#include <curses.h>
//#include <asio.hpp>
#include <json11.hpp>

#include <tfm.h>
#include <tomcrypt.h>

#pragma warning(pop)

// Version number
#include <version.hpp>

#include "Utils\CSV.hpp"
#include "Utils\Utils.hpp"
#include "Utils\WebIO.hpp"
#include "Utils\Memory.hpp"
#include "Utils\Hooking.hpp"
#include "Utils\Compression.hpp"

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

// Revision number
#define STRINGIZE_(x) #x
#define STRINGIZE(x) STRINGIZE_(x)

#define MILESTONE "beta"

#define REVISION_STR STRINGIZE(REVISION)
#define VERSION 4,2,REVISION
#define VERSION_STR "4.2." REVISION_STR

#define Assert_Size(x, size) static_assert(sizeof(x) == size, STRINGIZE(x) " structure has an invalid size.")
