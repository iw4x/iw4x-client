#pragma once

// Vulkan API, as the renderer uses it.
//
// We call Vulkan through volk, which loads vulkan-1.dll at run time (see
// deps/premake/volk.lua), so that iw4x.dll doesn't import it. Note that
// volk.h declares the Win32 types that vulkan_win32.h needs (HWND and the
// like) without including <windows.h>.
//
#include <volk.h>
