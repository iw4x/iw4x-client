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
    connection ();

    std::string
    server_name ();

    std::string
    server_address ();

    std::string
    server_version ();

    std::filesystem::path
    base_path ();
  }
}
