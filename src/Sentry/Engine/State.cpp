#include "State.hpp"

#include "../Types.hpp"

#include "../../Components/Modules/Dedicated.hpp"
#include "../../Components/Modules/Dvar.hpp"
#include "../../Components/Modules/Network.hpp"
#include "../../Components/Modules/Party.hpp"
#include "../../Components/Modules/TextRenderer.hpp"
#include "../../Components/Modules/ZoneBuilder.hpp"

#include <version.hpp>

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
    architecture ()
    {
      return Utils::GetWindowsArchitecture ();
    }

    std::string
    launch_parameters ()
    {
      return Utils::String::Convert (Utils::GetLaunchParameters ());
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
    connection ()
    {
      if (!Game::CL_IsCgameInitialized ())
        return "menu";

      return Components::Dedicated::IsRunning () ? "private" : "server";
    }

    std::string
    server_name ()
    {
      return remote () ? sanitize (Components::Party::GetHostName ()) : std::string ();
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
  }
}
