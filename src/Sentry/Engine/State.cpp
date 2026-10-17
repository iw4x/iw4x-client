#include "State.hpp"

#include "../Types.hpp"

#include "../Redact.hpp"

#include "../../Components/Modules/Dedicated.hpp"
#include "../../Components/Modules/Dvar.hpp"
#include "../../Components/Modules/Maps.hpp"
#include "../../Components/Modules/Network.hpp"
#include "../../Components/Modules/Party.hpp"
#include "../../Components/Modules/TextRenderer.hpp"
#include "../../Components/Modules/ZoneBuilder.hpp"

#include <version.hpp>

#include <intrin.h>

namespace Sentry
{
  namespace engine
  {
    namespace
    {
      std::string
      read (const Game::dvar_t** d)
      {
        if (d == nullptr || *d == nullptr)
          return {};

        const char* v ((*d)->current.string);

        return v != nullptr ? std::string (v) : std::string ();
      }

      // Return the value of a dvar the way the console shows it, which also
      // covers the non-string types. Note that unlike Components::Dvar::Var we
      // don't register the dvar if it doesn't exist.
      //
      std::string
      displayed (const char* n)
      {
        const Game::dvar_t* d (Game::Dvar_FindVar (n));

        if (d == nullptr)
          return {};

        const char* v (Game::Dvar_DisplayableValue (d));

        return v != nullptr ? std::string (v) : std::string ();
      }

      // Return the graphics adapter. Since it doesn't change, we only ask
      // once Direct3D is up.
      //
      const std::optional<D3DADAPTER_IDENTIFIER9>&
      adapter ()
      {
        static std::optional<D3DADAPTER_IDENTIFIER9> r;

        if (r)
          return r;

        IDirect3D9* d (Game::d3d9 != nullptr ? *Game::d3d9 : nullptr);

        if (d == nullptr)
          return r;

        D3DADAPTER_IDENTIFIER9 i {};

        if (SUCCEEDED (d->GetAdapterIdentifier (D3DADAPTER_DEFAULT, 0, &i)))
          r = i;

        return r;
      }

      // Return true if this is one of the IWDs that the game (including its
      // DLC) or IW4x ships, such as iw_00, iw_dlc5_01, localized_english_iw00,
      // or iw4x_00.
      //
      bool
      stock (const std::string& n)
      {
        static const std::regex r (
          "iw_[0-9]{2}|iw_dlc[0-9]+_[0-9]{2}|iw4x_[0-9]{2}|"
          "localized_[a-z_]+_iw[0-9]{2}",
          std::regex::icase | std::regex::optimize);

        return std::regex_match (n, r);
      }

      std::string
      sanitize (const std::string& s)
      {
        if (s.empty ())
          return {};

        return Components::TextRenderer::StripAllTextIcons (
          Components::TextRenderer::StripColors (s));
      }

      bool
      remote ()
      {
        return Game::CL_IsCgameInitialized ()
          && !Components::Dedicated::IsRunning ();
      }
    }

    std::string
    client_version ()
    {
      std::string v (read (Game::shortversion));

      return !v.empty () ? v : std::string (REVISION_STR);
    }

    std::string
    build_configuration ()
    {
#ifdef _DEBUG
      return "debug";
#else
      return "release";
#endif
    }

    std::string
    branch ()
    {
      return GIT_BRANCH;
    }

    std::string
    operating_system ()
    {
      return Utils::IsWineEnvironment () ? "Wine" : Utils::GetWindowsVersion ();
    }

    std::string
    wine ()
    {
      return Utils::IsWineEnvironment () ? "yes" : "no";
    }

    std::string
    wine_version ()
    {
      HMODULE m (GetModuleHandleA ("ntdll.dll"));

      if (m == nullptr)
        return {};

      using version_function = const char* (CDECL*) ();
      using host_function = void (CDECL*) (const char**, const char**);

      auto v (reinterpret_cast<version_function> (
                GetProcAddress (m, "wine_get_version")));

      if (v == nullptr)
        return {};

      std::string r (v ());

      // The host is what Wine runs on, such as Linux with its kernel version.
      //
      auto h (reinterpret_cast<host_function> (
                GetProcAddress (m, "wine_get_host_version")));

      if (h != nullptr)
      {
        const char* s (nullptr);
        const char* l (nullptr);

        h (&s, &l);

        if (s != nullptr && l != nullptr)
          r += std::format (" on {} {}", s, l);
      }

      return r;
    }

    std::string
    architecture ()
    {
      return Utils::GetWindowsArchitecture ();
    }

    std::string
    launch_parameters ()
    {
      int n (0);
      wchar_t** v (CommandLineToArgvW (GetCommandLineW (), &n));

      if (v == nullptr)
        return {};

      std::vector<std::string> as;

      for (int i (1); i < n; ++i)
        as.push_back (Utils::String::Convert (v[i]));

      LocalFree (v);

      // A server is commonly started with its rcon password on the command
      // line, which is not something to send along.
      //
      return redact_arguments (as);
    }

    std::string
    install_path ()
    {
      return base_path ().string ();
    }

    std::string
    role ()
    {
      if (Components::ZoneBuilder::IsEnabled ())
        return "zonebuilder";

      if (Components::Dedicated::IsEnabled ())
        return "dedicated";

      return "client";
    }

    std::string
    mod ()
    {
      const std::string m (sanitize (read (Game::fs_gameDirVar)));

      return !m.empty () ? m : std::string ("none");
    }

    std::string
    map ()
    {
      return read (Game::sv_mapname);
    }

    std::string
    gametype ()
    {
      return read (Game::sv_gametype);
    }

    std::string
    language ()
    {
      const char* l (Game::Win_GetLanguage ());

      return l != nullptr ? std::string (l) : std::string ();
    }

    std::string
    usermap ()
    {
      // Note that IsUserMap() goes to the filesystem, so only ask again once
      // the map changes.
      //
      static std::string last;
      static std::string r;

      const std::string m (map ());

      if (m != last)
      {
        last = m;
        r = m.empty () ? "" : Components::Maps::IsUserMap (m) ? "yes" : "no";
      }

      return r;
    }

    // Return the IWDs that are loaded but that neither the game nor IW4x
    // ships, such as the ones of a mod or of a user map, or the ones that
    // the player dropped into the game directories.
    //
    std::string
    custom_iwds ()
    {
      // We are asked twice for every publish of the scope (see modded()), so
      // reuse what we found for a moment rather than walk the search paths
      // again.
      //
      static std::chrono::steady_clock::time_point at;
      static std::string r;

      const auto now (std::chrono::steady_clock::now ());

      if (now - at < std::chrono::seconds (1))
        return r;

      at = now;
      r.clear ();

      if (Game::fs_searchpaths == nullptr)
        return r;

      for (const Game::searchpath_s* s (*Game::fs_searchpaths);
           s != nullptr;
           s = s->next)
      {
        if (s->iwd == nullptr || stock (s->iwd->iwdBasename))
          continue;

        if (!r.empty ())
          r += ", ";

        r += std::format ("{}/{}", s->iwd->iwdGamename, s->iwd->iwdBasename);
      }

      return r;
    }

    std::string
    modded ()
    {
      const bool m (!read (Game::fs_gameDirVar).empty ()
                    || usermap () == "yes"
                    || !custom_iwds ().empty ());

      return m ? "yes" : "no";
    }

    std::string
    connection ()
    {
      if (!Game::CL_IsCgameInitialized ())
        return "menu";

      return Components::Dedicated::IsRunning () ? "private" : "server";
    }

    std::string
    server_name ()
    {
      if (remote ())
        return sanitize (Components::Party::GetHostName ());

      // We are hosting the server ourselves. Note that IsRunning() expects
      // sv_running to be registered, which is not yet the case when we first
      // publish the scope.
      //
      if (*Game::sv_running != nullptr && Components::Dedicated::IsRunning ())
        return sanitize (displayed ("sv_hostname"));

      return {};
    }

    std::string
    server_address ()
    {
      if (!remote ())
        return {};

      return Components::Network::Address (*Game::connectedHost).getString ();
    }

    std::string
    server_version ()
    {
      if (!remote ())
        return {};

      return Components::Dvar::Var ("sv_version").get<std::string> ();
    }

    std::string
    display_mode ()
    {
      return displayed ("r_mode");
    }

    std::string
    fullscreen ()
    {
      return displayed ("r_fullscreen");
    }

    std::string
    refresh_rate ()
    {
      return displayed ("r_displayRefresh");
    }

    std::string
    vsync ()
    {
      return displayed ("r_vsync");
    }

    std::string
    max_fps ()
    {
      return displayed ("com_maxfps");
    }

    std::string
    gpu ()
    {
      const auto a (adapter ());

      return a ? std::string (a->Description) : std::string ();
    }

    std::string
    gpu_driver ()
    {
      const auto a (adapter ());

      if (!a)
        return {};

      const LARGE_INTEGER& v (a->DriverVersion);

      return std::format ("{}.{}.{}.{}",
                          HIWORD (v.HighPart), LOWORD (v.HighPart),
                          HIWORD (v.LowPart), LOWORD (v.LowPart));
    }

    std::string
    cpu ()
    {
      static const std::string r ([]
      {
        int i[4] {};
        __cpuid (i, 0x80000000);

        if (static_cast<unsigned> (i[0]) < 0x80000004)
          return std::string ();

        // The brand string is spread over three leaves, 16 bytes each.
        //
        char b[49] {};

        for (int l (0); l != 3; ++l)
          __cpuid (reinterpret_cast<int*> (b + l * 16), 0x80000002 + l);

        std::string s (b);

        s.erase (0, s.find_first_not_of (' '));
        s.erase (s.find_last_not_of (' ') + 1);

        return s;
      } ());

      return r;
    }

    std::string
    processors ()
    {
      const unsigned n (std::jthread::hardware_concurrency ());

      return n != 0 ? std::to_string (n) : std::string ();
    }

    std::string
    memory ()
    {
      MEMORYSTATUSEX m {};
      m.dwLength = sizeof (m);

      if (GlobalMemoryStatusEx (&m) == FALSE)
        return {};

      return std::format ("{} MB", m.ullTotalPhys / (1024 * 1024));
    }

    std::filesystem::path
    base_path ()
    {
      const char* p (Game::Sys_DefaultInstallPath ());

      if (p != nullptr && p[0] != '\0')
        return std::filesystem::path (p);

      const std::string c (read (Game::fs_basepath));

      if (!c.empty ())
        return std::filesystem::path (c);

      std::error_code ec;
      std::filesystem::path w (std::filesystem::current_path (ec));

      return ec ? std::filesystem::path (".") : w;
    }

    std::filesystem::path
    stats_path ()
    {
      // See Components::Stats::SprintfLiveStorageFilename().
      //
      const std::string m (read (Game::fs_gameDirVar));
      const std::filesystem::path p (base_path () / "players");

      return m.empty ()
        ? p / BASEGAME_LEGACY ".stat"
        : p / m / BASEGAME_LEGACY ".stat";
    }

    std::filesystem::path
    config_path ()
    {
      return base_path () / "players" / CLIENT_CONFIG;
    }
  }
}
