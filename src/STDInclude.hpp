#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <wincrypt.h>
#include <time.h>
#include <timeapi.h>
#include <WinSock2.h>

#include <map>
#include <mutex>
#include <string>
#include <vector>
#include <sstream>
#include <utility>
#include <algorithm>
#include <regex>

#include <version.hpp>

#include "Utils\Utils.hpp"
#include "Utils\WebIO.hpp"
#include "Utils\Hooking.hpp"

#include "Steam\Steam.hpp"

#include "Game\Structs.hpp"
#include "Game\Functions.hpp"

#include "Components\Loader.hpp"

// Libraries
#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "Crypt32.lib")
#pragma comment(lib, "Ws2_32.lib")
