#pragma once

#include "sentry_sync.h"

// Use Win32 calling convention expected by CreateThread on 32-bit MinGW.
//
#undef SENTRY_THREAD_FN
#define SENTRY_THREAD_FN static DWORD WINAPI
