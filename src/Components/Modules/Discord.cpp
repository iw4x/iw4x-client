#include "Discord.hpp"
#include "Party.hpp"
#include "TextRenderer.hpp"

#include <discord_rpc.h>

namespace Components
{
  namespace
  {
    // The IW4x application on the Discord developer portal whose rich
    // presence art is the x86 maps' load screens.
    //
    constexpr auto ApplicationId = "1559602059727081602";

    // The maps whose art is uploaded to the application, by short name and
    // sorted. Discord draws nothing at all (not even the application icon)
    // for a key the application doesn't have, so we only name the art of
    // these maps and leave the image unset for the others, which Discord
    // then fills with the application icon.
    //
    constexpr std::string_view UploadedArt[] =
    {
      "mp_afghan",
      "mp_boneyard",
      "mp_brecourt",
      "mp_checkpoint",
      "mp_derail",
      "mp_estate",
      "mp_favela",
      "mp_highrise",
      "mp_invasion",
      "mp_nightshift",
      "mp_quarry",
      "mp_rundown",
      "mp_rust",
      "mp_subbase",
      "mp_terminal",
      "mp_underpass",
    };

    static_assert(std::ranges::is_sorted(UploadedArt), "keep the list sorted, it is searched by bisection");

    // How often we read the engine and the minimum interval between two
    // presence updates. Discord limits updates to a few every 20 seconds, so
    // we publish the latest presence at most this often and drop the ones
    // in between.
    //
    constexpr auto ObserveInterval = 500ms;
    constexpr auto PublishInterval = 5s;

    // The text fields' limit in bytes (see discord_rpc.h). Discord also
    // rejects a details or state text shorter than two characters.
    //
    constexpr std::size_t TextCapacity = 127;

    // What the player is doing, as read off the engine.
    //
    struct Scene
    {
      enum class Place
      {
        Menus,
        PrivateLobby,
        PrivateMatch,
        Server
      };

      Place where = Place::Menus;

      bool hardcore = false;

      // Display names in UTF-8 ("Team Deathmatch", "Terminal") and the map's
      // short name ("mp_terminal"), which names its art. Empty if there is
      // none.
      //
      std::string mode;
      std::string map;
      std::string mapKey;

      // On a remote server: its name, its address, and how many players it
      // has and holds.
      //
      std::string server;
      std::string address;
      int players = 0;
      int capacity = 0;

      // The end of a match with a time limit, in seconds since the Unix
      // epoch, or 0.
      //
      std::int64_t matchEnd = 0;

      bool operator==(const Scene&) const = default;
    };

    std::int64_t UnixNow()
    {
      return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    }

    // Return the engine text in UTF-8 without color codes and text icons, or
    // nothing if it isn't representable. The engine's text is single-byte
    // and Latin-1 in the Western locales, which we convert. In the other
    // locales its bytes mean something else, so the caller falls back to an
    // internal name.
    //
    std::optional<std::string> EngineTextToUtf8(const char* text)
    {
      if (!text || !*text) return {};

      const auto stripped = TextRenderer::StripAllTextIcons(TextRenderer::StripColors(std::string(text)));

      // English, French, German, Italian, Spanish, and British, in the
      // engine's language order.
      //
      const auto latin1 = Game::SEH_GetCurrentLanguage() <= 5;

      std::string result;
      result.reserve(stripped.size());

      for (const auto c : stripped)
      {
        const auto b = static_cast<unsigned char>(c);

        if (b < 0x20) continue;

        if (b < 0x80)
        {
          result += c;
        }
        else if (latin1)
        {
          result += static_cast<char>(0xC0 | (b >> 6));
          result += static_cast<char>(0x80 | (b & 0x3F));
        }
        else
        {
          return {};
        }
      }

      if (result.empty()) return {};
      return result;
    }

    // Return the text cut to fit a presence field without splitting a UTF-8
    // sequence.
    //
    std::string Truncate(std::string text)
    {
      if (text.size() <= TextCapacity) return text;

      auto size = TextCapacity;
      while (size && (static_cast<unsigned char>(text[size]) & 0xC0) == 0x80) --size;

      text.resize(size);
      return text;
    }

    // Return the map's short name ("mp_rust") from whatever names it. The
    // client game state holds the path of the loaded map
    // ("maps/mp/mp_rust.d3dbsp") while ui_mapname holds the short name.
    //
    std::string ShortMapName(std::string_view map)
    {
      if (const auto i = map.find_last_of("/\\"); i != std::string_view::npos) map.remove_prefix(i + 1);
      if (const auto i = map.find('.'); i != std::string_view::npos) map = map.substr(0, i);

      return std::string(map);
    }

    // Return the display name of the game type, or the internal name if the
    // game doesn't have one we can show.
    //
    std::string ModeName(const std::string& gametype)
    {
      if (gametype.empty()) return {};

      const auto* found = Game::UI_GetGameTypeDisplayName(gametype.data());
      if (found && std::strcmp(found, gametype.data()) != 0)
      {
        if (auto name = EngineTextToUtf8(found)) return *name;
      }

      return gametype;
    }

    // As above but for the map, whose fallback leaves out the "mp_" prefix.
    //
    std::string MapName(const std::string& map)
    {
      if (map.empty()) return {};

      const auto* found = Game::UI_GetMapDisplayName(map.data());
      if (found && std::strcmp(found, map.data()) != 0)
      {
        if (auto name = EngineTextToUtf8(found)) return *name;
      }

      return map.starts_with("mp_") ? map.substr(3) : map;
    }

    // Return when the match ends, in seconds since the Unix epoch, or 0 for a
    // match without a time limit.
    //
    // The game's HUD computes the time left as the match's end in server
    // time less the server time of the snapshot being moved to, and so do
    // we. We place the end on the wall clock once and only move it when the
    // match's end really moves (a pause or an extension). Otherwise the end
    // would jitter by a second between reads, which is a presence update for
    // nothing.
    //
    std::int64_t MatchEnd()
    {
      static std::int64_t held = 0;

      const auto& cgs = Game::cgsArray[0];
      const auto& cg = Game::cgArray[0];

      if (cgs.gameEndTime <= 0 || !cg.nextSnap) return held = 0;

      const auto remaining = cgs.gameEndTime - cg.nextSnap->serverTime;
      if (remaining <= 0) return held = 0;

      const auto end = UnixNow() + (remaining + 999) / 1000;

      if (!held || end < held - 2 || end > held + 2) held = end;
      return held;
    }

    Scene Observe()
    {
      Scene scene;

      if (Game::CL_IsCgameInitialized())
      {
        const auto& cgs = Game::cgsArray[0];

        scene.where = cgs.localServer ? Scene::Place::PrivateMatch : Scene::Place::Server;
        scene.hardcore = cgs.hardcore;
        scene.mapKey = ShortMapName(cgs.mapname);
        scene.mode = ModeName(cgs.gametype);
        scene.map = MapName(scene.mapKey);
        scene.matchEnd = MatchEnd();

        if (scene.where == Scene::Place::Server)
        {
          const auto target = Party::Target();

          // Only a server on the internet is one that friends can join, so
          // we leave out the address of any other.
          //
          if (target.getType() == Game::NA_IP && !target.isLocal())
          {
            scene.address = target.getString();
          }

          scene.server = EngineTextToUtf8(Party::GetHostName().data()).value_or("");
          scene.players = Game::cgArray[0].snap ? Game::cgArray[0].snap->numClients : 1;
          scene.capacity = Party::GetMaxClients();
        }
      }
      else if (Party::IsInLobby())
      {
        scene.where = Scene::Place::PrivateLobby;
        scene.mapKey = ShortMapName((*Game::ui_mapname)->current.string);
        scene.mode = ModeName((*Game::ui_gametype)->current.string);
        scene.map = MapName(scene.mapKey);
      }

      return scene;
    }

    // Return "<mode> on <map>", with whichever of them there is.
    //
    std::string Activity(const Scene& scene)
    {
      auto mode = scene.mode;
      if (!mode.empty() && scene.hardcore) mode = "Hardcore " + mode;

      if (mode.empty()) return scene.map;
      if (scene.map.empty()) return mode;

      return mode + " on " + scene.map;
    }

    // The party id groups the friends on one server, so it is the same for
    // everybody there. We hash the address since the id is visible to anyone
    // who can see the presence while the join secret is only handed to
    // those who join.
    //
    std::string PartyId(const std::string& address)
    {
      return Utils::Cryptography::SHA256::Compute(address, true).substr(0, 32);
    }

    // The presence as last published and the scene it was built from. The
    // presence fields point into the strings below, which live until the
    // next build.
    //
    struct Publication
    {
      Scene scene;
      std::int64_t since = 0;

      std::string details;
      std::string state;
      std::string imageKey;
      std::string imageText;
      std::string partyId;
      std::string joinSecret;

      DiscordRichPresence presence{};
    };

    bool Running = false;

    // Whether Discord lacks our presence: initially and after it reconnects,
    // since it forgets the presence of a client that disconnects.
    //
    bool Stale = true;

    Publication Published;
    std::chrono::steady_clock::time_point LastPublish{};

    void Build(Publication& p, const Scene& scene)
    {
      // Start the timer over whenever the player gets somewhere else.
      //
      if (!p.since || scene.where != p.scene.where || scene.mapKey != p.scene.mapKey || scene.address != p.scene.address)
      {
        p.since = UnixNow();
      }

      p.scene = scene;

      switch (scene.where)
      {
      case Scene::Place::Menus:
        p.details = "In the menus";
        p.state.clear();
        break;
      case Scene::Place::PrivateLobby:
        p.details = Activity(scene);
        p.state = "In a private match lobby";
        break;
      case Scene::Place::PrivateMatch:
        p.details = Activity(scene);
        p.state = "In a private match";
        break;
      case Scene::Place::Server:
        p.details = Activity(scene);
        p.state = scene.server.empty() ? "On a server" : scene.server;
        break;
      }

      if (p.details.size() < 2) p.details = "Playing";

      p.details = Truncate(std::move(p.details));
      p.state = Truncate(std::move(p.state));

      p.imageKey.clear();
      p.imageText.clear();

      if (std::ranges::binary_search(UploadedArt, Utils::String::ToLower(scene.mapKey)))
      {
        p.imageKey = Utils::String::ToLower(scene.mapKey);
        p.imageText = Truncate(scene.map);
      }

      p.partyId.clear();
      p.joinSecret.clear();

      if (!scene.address.empty())
      {
        p.partyId = PartyId(scene.address);
        p.joinSecret = scene.address;
      }

      auto& r = p.presence;
      r = {};

      r.details = p.details.data();
      r.state = p.state.size() >= 2 ? p.state.data() : nullptr;

      // Count down to the end of a match with a time limit and otherwise up
      // from when the player got where they are. Discord shows one or the
      // other.
      //
      if (scene.matchEnd > UnixNow())
      {
        r.endTimestamp = scene.matchEnd;
      }
      else
      {
        r.startTimestamp = p.since;
      }

      if (!p.imageKey.empty())
      {
        r.largeImageKey = p.imageKey.data();
        r.largeImageText = p.imageText.size() >= 2 ? p.imageText.data() : nullptr;
      }

      if (!p.partyId.empty())
      {
        r.partyId = p.partyId.data();
        r.partySize = std::clamp(scene.players, 1, 64);
        r.partyMax = scene.capacity > 0 ? std::clamp(scene.capacity, r.partySize, 64) : 0;
        r.partyPrivacy = DISCORD_PARTY_PUBLIC;
        r.joinSecret = p.joinSecret.data();
      }

      r.instance = 1;
    }

    void Update()
    {
      if (!Running) return;

      Discord_RunCallbacks();

      const auto scene = Observe();
      if (scene == Published.scene && !Stale) return;

      // Publish the latest scene once the interval has passed. The scenes in
      // between are never published.
      //
      const auto now = std::chrono::steady_clock::now();
      if (now - LastPublish < PublishInterval) return;

      Build(Published, scene);
      Stale = false;
      LastPublish = now;

      Discord_UpdatePresence(&Published.presence);
    }

    void Ready(const DiscordUser* user)
    {
      Logger::Print("Discord: Connected as {}\n", user && user->username ? user->username : "unknown");

      Stale = true;
      LastPublish = {};
    }

    void Errored(const int errorCode, const char* message)
    {
      Logger::Debug("Discord: Error ({}): {}\n", errorCode, message ? message : "");
    }

    // Join the server a friend is on. The secret comes from Discord, so we
    // only accept an internet address and connect to it as parsed. We never
    // pass the secret itself to the console, where it could carry commands.
    //
    void JoinGame(const char* secret)
    {
      if (!secret) return;

      const Network::Address target{std::string(secret)};

      if (!target.isValid() || target.getType() != Game::NA_IP || target.isLocal())
      {
        Logger::Print("Discord: Ignoring join request for invalid server '{}'\n", secret);
        return;
      }

      Party::Connect(target);
    }

    // We publish public parties only, which friends join without asking,
    // so a request to join is not something we expect. Ignore it.
    //
    void JoinRequest(const DiscordUser* request)
    {
      if (request)
      {
        Discord_Respond(request->userId, DISCORD_REPLY_IGNORE);
      }
    }

    void Shutdown()
    {
      if (!Running) return;

      Running = false;

      Discord_ClearPresence();
      Discord_Shutdown();
    }
  }

  Discord::Discord()
  {
    if (Dedicated::IsEnabled() || ZoneBuilder::IsEnabled())
    {
      return;
    }

    DiscordEventHandlers handlers{};
    handlers.ready = Ready;
    handlers.errored = Errored;
    handlers.disconnected = Errored;
    handlers.joinGame = JoinGame;
    handlers.joinRequest = JoinRequest;

    Discord_Initialize(ApplicationId, &handlers, 1, nullptr);
    Running = true;

    Scheduler::Loop(Update, Scheduler::Pipeline::MAIN, ObserveInterval);
    Scheduler::OnGameShutdown(Shutdown);
  }
}
