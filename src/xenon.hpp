#pragma once

#include <format>
#include <string_view>

//
// xenon.hpp — Project Xenon Central Configuration
//
// Naming convention: __XENON_<FEATURE_NAME>
// Comment out a define to disable a feature without removing code.
//

// ============================================================
//  Build Tags / Versioning
// ============================================================

#define __XENON_BUILD_DEV
#define __XENON_VERSION "0.1"

// ============================================================
//  Core Feature Flags
// ============================================================
#define __XENON_UI_BIGFONT 0.58333331f
#define __XENON_UI_SMALLFONT 0.375f
#define __XENON_UI_BINDS
#define __XENON_UI_THUMBSTICK
#define __XENON_UI_SPECTATOR
#define __XENON_INPUT_SPECTATOR
#define __XENON_SCOREBOARD // TODO: Buttons text
#define __XENON_GPAD
#define __LANG_TEST

#define __XENON_LADDER_MOVEMENT
#define __XENON_SPRINT_INPUT

// ============================================================
//  Unrelated to the Xenon port
// ============================================================
#define __JSON_FONTLOADER



// ============================================================
//  Debug / Logging Controls
// ============================================================

// When defined, XLOG prefixes output with ^2[XENON]: (green) for easy filtering.
// Without it, XLOG behaves identically to a plain Logger::Print call.
#define __XENON_LOGGING

#ifndef RC_INVOKED

// ============================================================
//  PATCH_DVAR(name, field, value)
//
//  Silently forces a dvar to a fixed value (current/latched/reset)
//  without triggering the modified flag.
//  field is the DvarValue union member: integer, value, enabled, string
// ============================================================
#define PATCH_DVAR(name, field, val)                  \
    do {                                               \
        if (auto* _d = Game::Dvar_FindVar(name))      \
        {                                              \
            _d->current.field = (val);                \
            _d->latched.field = (val);                \
            _d->reset.field   = (val);                \
            _d->modified      = false;                \
        }                                              \
    } while (0)

#ifdef __XENON_LOGGING
namespace xenon
{
    template<typename... Args>
    inline void Log(const std::string_view& fmt, Args&&... args)
    {
        const auto msg = std::vformat(fmt, std::make_format_args(args...));
        Components::Logger::Print("^2[XENON]: {}", msg);
    }
}
#define XLOG(fmt, ...) xenon::Log(fmt, ##__VA_ARGS__)
#else
#define XLOG(fmt, ...) Components::Logger::Print(fmt, ##__VA_ARGS__)
#endif
#endif

// #define __XENON_DEBUG_UI
// #define __XENON_VERBOSE_LOGGING
