#pragma once

#include "../Types.hpp"

namespace Sentry
{
  namespace engine
  {
    std::string
    client_version ();

    std::string
    build_configuration ();

    std::string
    branch ();

    std::string
    operating_system ();

    std::string
    wine ();

    std::string
    wine_version ();

    std::string
    architecture ();

    std::string
    launch_parameters ();

    std::string
    install_path ();

    std::string
    role ();

    std::string
    mod ();

    std::string
    map ();

    std::string
    gametype ();

    std::string
    language ();

    std::string
    usermap ();

    std::string
    custom_iwds ();

    std::string
    modded ();

    std::string
    connection ();

    std::string
    server_name ();

    std::string
    server_address ();

    std::string
    server_version ();

    std::string
    display_mode ();

    std::string
    fullscreen ();

    std::string
    refresh_rate ();

    std::string
    vsync ();

    std::string
    max_fps ();

    std::string
    gpu ();

    std::string
    gpu_driver ();

    std::string
    cpu ();

    std::string
    processors ();

    std::string
    memory ();

    std::filesystem::path
    base_path ();

    // Return the stats file of the current mod.
    //
    std::filesystem::path
    stats_path ();

    std::filesystem::path
    config_path ();
  }
}
