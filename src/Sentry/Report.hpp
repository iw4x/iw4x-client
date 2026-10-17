#pragma once

#include "Types.hpp"

#include "Options.hpp"

namespace Sentry
{
  // Offer the player to send a report about a problem that ends the game
  // without crashing it, such as fastfiles that failed to convert.
  //
  // The SDK is not necessarily running at that point and, even if it were, it
  // would send the report without asking. So we write the envelope ourselves
  // and hand it to the crash reporter, the same way the crash daemon does. The
  // files are attached as is, except for the ones that are too large for
  // Sentry to accept.
  //
  // Return false if reporting is off or there is no reporter to hand it to.
  //
  bool
  offer (const options&,
         const std::string& logger,
         const std::string& message,
         std::span<const std::filesystem::path> attachments);
}
