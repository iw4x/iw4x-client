#include <Utils/Compression.hpp>

#include <proto/party.pb.h>

#include "Playlist.hpp"
#include "Party.hpp"

namespace Components
{
  std::string Playlist::CurrentPlaylistBuffer;
  std::string Playlist::ReceivedPlaylistBuffer;
  std::unordered_map<const void*, std::string> Playlist::MapRelocation;

  void Playlist::LoadPlaylist()
  {
    // Check if playlist already loaded
    if (*Game::s_havePlaylists) return;

    // Don't load playlists when dedi and no party
    if (Dedicated::IsEnabled() && !Party::IsEnabled())
    {
      *Game::s_havePlaylists = true;
      Dvar::Var("xblive_privateserver").set(true);
      return;
    }

    Dvar::Var("xblive_privateserver").set(false);

    const auto playlistFilename = Dvar::Var("playlistFilename").get<std::string>();
    FileSystem::File playlist(playlistFilename);

    if (playlist.exists())
    {
      Logger::Print("Parsing playlist '{}'...\n", playlist.getName());
      Game::Playlist_ParsePlaylists(playlist.getBuffer().data());
      *Game::s_havePlaylists = true;
    }
    else
    {
      Logger::Print("Unable to load playlist '{}'!\n", playlist.getName());
    }
  }

  // On x86 we jump to LoadPlaylist() at the end of Live_Init(). There x64
  // sets a flag after its playlist download instead, so we load ours in
  // place of the check whether the playlist fastfile exists. And we say that
  // it does, so that the game doesn't go and try to download it. A bit of a
  // white lie, but a harmless one since we disable the download anyway.
  //
  bool Playlist::LoadPlaylist_Hk()
  {
    LoadPlaylist();
    return true;
  }

  char* Playlist::Com_ParseOnLine_Hk(const char** data_p)
  {
    MapRelocation.clear();
    CurrentPlaylistBuffer = Utils::Compression::ZLib::Compress(*data_p);
    return Game::Com_ParseOnLine(data_p);
  }

  void Playlist::PlaylistRequest(const Network::Address& address, [[maybe_unused]] const std::string& data)
  {
    // Addresses that recently sent a wrong password, with the time they did.
    // We throttle them so that this can't be used as a fast password oracle.
    //
    static std::unordered_map<std::uint32_t, int> failedRequests;
    constexpr auto failedRequestTimeout = 1000;

    const auto* password = *Game::g_password ? (*Game::g_password)->current.string : "";

    if (*password)
    {
      const auto time = Game::Sys_Milliseconds();
      const auto ip = address.getIP().full;

      if (const auto itr = failedRequests.find(ip); itr != failedRequests.end() && (time - itr->second) < failedRequestTimeout)
      {
        return; // Flooding.
      }

      if (!Utils::Cryptography::CompareConstantTime(password, data))
      {
        std::erase_if(failedRequests, [&](const auto& entry) { return (time - entry.second) >= failedRequestTimeout; });
        failedRequests[ip] = time;

        Logger::PrintFail2Ban("Invalid packet from IP address: {}\n", Network::AdrToString(address));
        Network::SendCommand(address, "playlistInvalidPassword");
        return;
      }
    }

    Logger::Print("Received playlist request, sending currently stored buffer.\n");

    std::string compressedList = CurrentPlaylistBuffer;

    Proto::Party::Playlist list;
    list.set_hash(Utils::Cryptography::JenkinsOneAtATime::Compute(compressedList));
    list.set_buffer(compressedList);

    Network::SendCommand(address, "playlistResponse", list.SerializeAsString());
  }

  void Playlist::PlaylistResponse(const Network::Address& address, [[maybe_unused]] const std::string& data)
  {
    if (!Party::PlaylistAwaiting())
    {
      Logger::Print("Received stray playlist response, ignoring it.\n");
      return;
    }

    if (address != Party::Target())
    {
      Logger::Print("Received playlist from someone else than our target host, ignoring it.\n");
      return;
    }

    Proto::Party::Playlist list;

    if (!list.ParseFromString(data))
    {
      Party::PlaylistError(std::format("Received playlist response from {}, but it is invalid.", address.getString()));
      ReceivedPlaylistBuffer.clear();
    }
    else
    {
      // Generate buffer and hash
      const auto& compressedData = list.buffer();
      const auto hash = Utils::Cryptography::JenkinsOneAtATime::Compute(compressedData);

      // Validate hashes
      if (hash != list.hash())
      {
        Party::PlaylistError(std::format("Received playlist response from {}, but the checksum did not match ({} != {}).", address.getString(), list.hash(), hash));
        ReceivedPlaylistBuffer.clear();
        return;
      }

      // Decompress buffer
      ReceivedPlaylistBuffer = Utils::Compression::ZLib::Decompress(compressedData);

      // Load and continue connection
      Logger::Print("Received playlist, loading and continuing connection...\n");
      Game::Playlist_ParsePlaylists(ReceivedPlaylistBuffer.data());
      Party::PlaylistContinue();
    }
  }

  void Playlist::PlaylistInvalidPassword([[maybe_unused]] const Network::Address& address, [[maybe_unused]] const std::string& data)
  {
    Party::PlaylistError("Error: Invalid Password for Party.");
  }

  void Playlist::MapNameCopy(char* dest, const char* src, int destsize)
  {
    Game::I_strncpyz(dest, src, destsize);
    MapRelocation[dest] = src;
  }

  void Playlist::SetMapName(const char* dvarName, const char* value)
  {
    auto i = MapRelocation.find(value);
    if (i != MapRelocation.end())
    {
      value = i->second.data();
    }

    Game::Dvar_SetStringByName(dvarName, value);
  }

  int Playlist::GetMapIndex(const char* mapname)
  {
    auto i = MapRelocation.find(mapname);
    if (i != MapRelocation.end())
    {
      mapname = i->second.data();
    }

    return Game::Live_GetMapIndex(mapname);
  }

  Playlist::Playlist()
  {
    // Default playlists
    //
    // Note that x64 defaults to "playlists.patch2", which it loads through a
    // RIP-relative operand, so we place our string where that can reach.
    //
    Utils::Hook::PlaceString(0x1401F4D31, 7, 0x14038B958, "playlists_default.info");

    // Disable playlist download function
    Utils::Hook::Set<BYTE>(0x1402A7140, 0xC3);

    // Note that on x86 we also keep Win_LoadPlaylistFastfile() from deleting
    // the playlist, which is moot since we disable that function below
    // anyway.
    //

    // Disable Playlist_ValidatePlaylistNum
    Utils::Hook::Set<BYTE>(0x14025A5D0, 0xC3);

    // Disable playlist checking
    Utils::Hook::Set<BYTE>(0x14010CF8E, 0xEB); // Too new
    Utils::Hook::Set<BYTE>(0x14010CF73, 0xEB); // Too old

    // Got playlists is true
    //Utils::Hook::Set<bool>(0x141BDDCE4, true);

    Utils::Hook(0x140110AFC, GetMapIndex, HOOK_CALL).install()->quick();
    Utils::Hook(0x14025A0C1, MapNameCopy, HOOK_CALL).install()->quick();
    Utils::Hook(0x14025A4DC, SetMapName, HOOK_CALL).install()->quick();

    // Store playlist buffer on load
    Utils::Hook(0x140259883, Com_ParseOnLine_Hk, HOOK_CALL).install()->quick(); // Playlist_ParsePlaylists

    //if (Dedicated::IsDedicated())
    {
      // Custom playlist loading
      Utils::Hook(0x1402A23E2, LoadPlaylist_Hk, HOOK_CALL).install()->quick(); // Live_Init

      // disable playlist.ff loading function (Win_LoadPlaylistFastfile)
      Utils::Hook::Set<std::uint8_t>(0x1402A74F0, 0xC3);
    }

    Network::OnClientPacket("getPlaylist", PlaylistRequest);
    Network::OnClientPacket("playlistResponse", PlaylistResponse);
    Network::OnClientPacket("playlistInvalidPassword", PlaylistInvalidPassword);
  }
}
