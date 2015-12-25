#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <wincrypt.h>
#include <time.h>
#include <timeapi.h>

#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "Crypt32.lib")

#include <map>
#include <mutex>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <utility>

#include "Game\Structs.hpp"
#include "Game\Functions.hpp"

#include "Utils\Utils.hpp"
#include "Utils\Hooking.hpp"

#include "Steam\Steam.hpp"

#include "Components\Loader.hpp"
