#pragma once

namespace Components
{
  // Discord rich presence.
  //
  // We read what the player is doing off the engine on the main thread and
  // publish it to the Discord client through discord-rpc whenever it
  // changes. The presence names the mode and the map, shows the map's load
  // screen (for the maps whose art is uploaded to the Discord application),
  // and counts down to the end of a match that has a time limit. On a
  // remote server it also publishes a party, which groups friends on the
  // same server and lets them join it from Discord.
  //
  class Discord : public Component
  {
  public:
    Discord();

    static std::string GetDiscordServerLink() { return "https://iw4x.io/discord"; }
  };
}
