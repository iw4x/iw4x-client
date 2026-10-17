#pragma once

#include "Types.hpp"

#include "Options.hpp"

namespace Sentry
{
  // Return a transport that, instead of sending an event, offers it to the
  // player through the crash reporter, the same way the crash daemon does for
  // crashes. Anything without an event (such as client reports) is dropped.
  // This is what makes the player agree to everything that leaves their
  // machine, crash or not.
  //
  // The options must outlive the transport.
  //
  sentry_transport_t*
  make_transport (const options&);

  // Offer the player to send a report about a problem that ends the game
  // without crashing it, such as fastfiles that failed to convert.
  //
  // This is for when the SDK is not running (yet), so we write the envelope
  // ourselves and hand it to the crash reporter. The files are attached as
  // is, except for the ones that are too large for Sentry to accept.
  //
  // Return false if reporting is off or there is no reporter to hand it to.
  //
  bool
  offer (const options&,
         const std::string& logger,
         const std::string& message,
         std::span<const std::filesystem::path> attachments);
}
