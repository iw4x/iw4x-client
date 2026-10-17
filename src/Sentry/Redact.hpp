#pragma once

#include "Types.hpp"

namespace Sentry
{
  // Return true if the text may hold a password, such as a password dvar, an
  // rcon command, or the SOCKS proxy settings.
  //
  bool
  sensitive (std::string_view);

  // Replace every sensitive line, keeping its line ending. We replace the
  // whole line since we can't tell where in, say, a bind the password is.
  //
  std::string
  redact_lines (std::string);

  // Replace the values that follow a sensitive argument, up to the next
  // command (+...) or option (-...). This covers both +set rcon_password x
  // and +rcon login x.
  //
  std::string
  redact_arguments (const std::vector<std::string>&);
}
