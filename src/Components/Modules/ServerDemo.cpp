#include "ServerDemo.hpp"

#include "Dedicated.hpp"
#include "Events.hpp"
#include "FileSystem.hpp"
#include "Scheduler.hpp"
#include "TextRenderer.hpp"

namespace Components
{
  std::array<ServerDemo::ClientData, Game::MAX_CLIENTS> ServerDemo::Clients;

  int ServerDemo::LastServerId = 0;
  bool ServerDemo::HaveServerId = false;

  Dvar::Var ServerDemo::SVDemoAutoRecord;
  Dvar::Var ServerDemo::SVDemosKeep;
  Dvar::Var ServerDemo::SVDemoBufferSize;
  Dvar::Var ServerDemo::SVDemoMaxFileSize;

  bool ServerDemo::ValidClientNum(const int clientNum)
  {
    return clientNum >= 0 && clientNum < static_cast<int>(Game::MAX_CLIENTS);
  }

  bool ServerDemo::HasMapChanged()
  {
    const auto serverId = *Game::sv_serverId_value;

    if (!HaveServerId)
    {
      LastServerId = serverId;
      HaveServerId = true;
      return false;
    }

    const auto mapChanged = ((serverId ^ LastServerId) & 0xF0) != 0;
    LastServerId = serverId;
    return mapChanged;
  }

  std::size_t ServerDemo::MaxBufferBytes()
  {
    return static_cast<std::size_t>(SVDemoBufferSize.get<int>()) * 1024;
  }

  std::size_t ServerDemo::MaxFileBytes()
  {
    const auto mib = SVDemoMaxFileSize.get<int>();
    if (mib <= 0) return std::numeric_limits<std::uint32_t>::max();
    return static_cast<std::size_t>(mib) * 1024 * 1024;
  }

  void ServerDemo::WriteMetadata(const ClientData& data, const std::string& baseName)
  {
    std::tm tm{};
    localtime_s(&tm, &data.recordStartTimeStamp);
    char dateBuf[64]{};
    asctime_s(dateBuf, sizeof(dateBuf), &tm);

    // Sys_Milliseconds wraps after about 24.8 days; unsigned subtraction keeps the
    // elapsed time right across the wrap.
    const auto lengthMs = static_cast<int>(
      static_cast<unsigned int>(Game::Sys_Milliseconds()) - static_cast<unsigned int>(data.recordStartMs));

    // Theatre::LoadDemos requires author, mapname, gametype, length and timestamp, and a
    // missing one throws an exception it does not catch. The author is the server that
    // recorded the demo; the player is recordedClient.
    const auto serverName = Dvar::Var("sv_hostname").get<std::string>();

    const nlohmann::json j
    {
      { "author", serverName },
      { "mapname", data.mapname },
      { "gametype", data.gametype },
      { "length", lengthMs },
      { "timestamp", std::to_string(data.recordStartTimeStamp) },
      { "date", dateBuf },
      { "map", data.mapname },
      { "mod", "" },
      { "revision", "iw4x-serverdemo-v2" },
      { "server", serverName },
      { "recordedClient", data.clientName },
    };

    FileSystem::FileWriter meta(std::format("{}{}.dm_13.json", DEMO_DIR, baseName));
    meta.write(j.dump());
  }

  namespace
  {
    // Unix timestamp at the end of "<anything>_<timestamp><ext>", or nothing if the name
    // does not have that shape.
    std::optional<long long> TrailingTimestamp(std::string_view name, const std::string_view ext)
    {
      if (!name.ends_with(ext)) return {};
      name.remove_suffix(ext.size());

      const auto sep = name.rfind('_');
      if (sep == std::string_view::npos || sep + 1 == name.size()) return {};

      const std::string digits(name.substr(sep + 1));
      char* end = nullptr;
      const auto value = std::strtoll(digits.data(), &end, 10);
      if (end != digits.data() + digits.size()) return {};

      return value;
    }
  }

  void ServerDemo::CleanupOldAutoDemos()
  {
    const auto keep = SVDemosKeep.get<int>();
    if (keep <= 0) return;

    // Names are auto_<gametype>_<map>_<player>_<timestamp>, so a plain name sort would
    // order by gametype, map and player first. Order by the trailing timestamp instead;
    // the player part may itself contain underscores, which is why it is read from the end.
    std::vector<std::pair<long long, std::string>> files;

    for (auto& demo : FileSystem::GetFileList(DEMO_DIR, "dm_13"))
    {
      if (!Utils::String::StartsWith(demo, AUTO_PREFIX)) continue;

      if (const auto timestamp = TrailingTimestamp(demo, ".dm_13"))
      {
        files.emplace_back(*timestamp, demo);
      }
    }

    std::ranges::sort(files);

    const auto numDel = static_cast<int>(files.size()) - keep;
    for (auto i = 0; i < numDel; ++i)
    {
      const auto& name = files[i].second;
      Logger::Print("[ServerDemo] Deleting old auto demo {}\n", name);
      FileSystem::_DeleteFile(DEMO_DIR, name);
      FileSystem::_DeleteFile(DEMO_DIR, std::format("{}.json", name));
    }
  }

  void ServerDemo::CloseFile(ClientData& data)
  {
    if (!data.fileActive) return;

    // End-of-demo marker, as CL_StopRecord writes it: a network record with sequence and
    // length both -1.
    constexpr unsigned char msgType = 0;
    constexpr int endMarker = -1;
    Game::FS_WriteToDemo(&msgType, sizeof(msgType), data.demoFile);
    Game::FS_WriteToDemo(&endMarker, sizeof(endMarker), data.demoFile);
    Game::FS_WriteToDemo(&endMarker, sizeof(endMarker), data.demoFile);

    WriteMetadata(data, data.fileBaseName);
    Game::FS_FCloseFile(data.demoFile);

    data.demoFile = 0;
    data.fileActive = false;
    data.fileBytes = 0;
    data.fileBaseName.clear();
  }

  void ServerDemo::ResetClient(const int clientNum, const bool wasMapChange)
  {
    if (!ValidClientNum(clientNum)) return;

    auto& data = Clients[clientNum];

    if (data.fileActive)
    {
      Logger::Print("[ServerDemo] {} client {} ({}), wrote {}{}.dm_13\n",
        wasMapChange ? "Rotated" : "Closed", clientNum, data.clientName, DEMO_DIR, data.fileBaseName);

      CloseFile(data);
    }

    data = ClientData{};
  }

  void ServerDemo::ResetAll(const bool wasMapChange)
  {
    for (auto i = 0; i < static_cast<int>(Game::MAX_CLIENTS); ++i)
    {
      ResetClient(i, wasMapChange);
    }
  }

  void ServerDemo::EnsureClientState(const int clientNum, const Game::client_s* cl)
  {
    auto& data = Clients[clientNum];

    if (!data.identityValid)
    {
      data.mapname = (*Game::sv_mapname)->current.string;
      data.gametype = (*Game::sv_gametype)->current.string;
      data.clientName = cl->name;
      data.identityValid = true;
    }

    if (data.bufferActive || data.bufferRetired) return;

    data.buffer.reserve(INITIAL_BUFFER_BYTES);
    data.bufferActive = true;
  }

  void ServerDemo::RetireBuffer(ClientData& data)
  {
    data.bufferActive = false;
    data.bufferRetired = true;

    // clear() keeps the capacity; shrink_to_fit releases it.
    data.buffer.clear();
    data.buffer.shrink_to_fit();
  }

  namespace
  {
    // Each record is built once and then written to the buffer and the file, so the two
    // cannot diverge.
    class RecordBuilder
    {
    public:
      void put(const void* p, const std::size_t n)
      {
        const auto* bytes = static_cast<const unsigned char*>(p);
        bytes_.insert(bytes_.end(), bytes, bytes + n);
      }

      template <typename T>
      void put(const T& value) { put(&value, sizeof(T)); }

      [[nodiscard]] const std::vector<unsigned char>& bytes() const { return bytes_; }

    private:
      std::vector<unsigned char> bytes_;
    };
  }

  void ServerDemo::EmitRecord(ClientData& data, const std::vector<unsigned char>& record)
  {
    if (record.empty()) return;

    if (data.bufferActive)
    {
      // A file must start at the gamestate, so the oldest records cannot be evicted to
      // make room. Past the limit the buffer is dropped; an open file is unaffected.
      if (data.buffer.size() + record.size() > MaxBufferBytes())
      {
        Logger::Print("[ServerDemo] Buffered history for a client hit the sv_demoBufferSize cap ({} KiB) - "
          "buffering stopped for this connection; serverrecord will need a reconnect (or use sv_demoAutoRecord)\n",
          SVDemoBufferSize.get<int>());
        RetireBuffer(data);
      }
      else
      {
        data.buffer.insert(data.buffer.end(), record.begin(), record.end());
      }
    }

    if (data.fileActive)
    {
      // A partial record misframes everything after it, so the demo ends here.
      const auto written = Game::FS_WriteToDemo(record.data(), static_cast<int>(record.size()), data.demoFile);
      if (written != static_cast<int>(record.size()))
      {
        const auto name = data.fileBaseName;

        CloseFile(data);
        data.autoRecordSuppressed = true;

        Logger::PrintError(Game::CON_CHANNEL_ERROR,
          "[ServerDemo] Short write to {}{}.dm_13 ({} of {} bytes) - closed, recording stopped for this "
          "connection\n", DEMO_DIR, name, written, record.size());
        return;
      }

      data.fileBytes += record.size();

      // A demo cannot be split, as a second file would not start at a gamestate, so
      // reaching the limit ends recording for the connection.
      if (data.fileBytes >= MaxFileBytes())
      {
        const auto name = data.fileBaseName;

        CloseFile(data);
        data.autoRecordSuppressed = true;

        Logger::Print("[ServerDemo] {}{}.dm_13 hit sv_demoMaxFileSize ({} MiB) - closed, and recording for this "
          "connection has stopped (a demo cannot be split: it must start at a gamestate)\n",
          DEMO_DIR, name, SVDemoMaxFileSize.get<int>());
      }
    }
  }

  void ServerDemo::AppendArchiveRecord(ClientData& data, const Game::client_s* cl)
  {
    // gentity->client is not set until the player has spawned.
    if (!cl->gentity || !cl->gentity->client) return;

    const auto* ps = &cl->gentity->client->ps;

    // Read by the demo reader as: index, origin, velocity, movementDir, bobCycle,
    // commandTime, viewangles, locationSelectionInfo. Raw, not compressed.
    RecordBuilder rec;

    constexpr unsigned char msgType = 1;
    rec.put(msgType);
    rec.put(data.archiveIndex);
    rec.put(ps->origin, sizeof(ps->origin));
    rec.put(ps->velocity, sizeof(ps->velocity));
    rec.put(ps->movementDir);
    rec.put(ps->bobCycle);
    rec.put(ps->commandTime);
    rec.put(ps->viewangles, sizeof(ps->viewangles));

    // Always 0, as in Theatre's client-side recorder. A non-zero value would require
    // another 8 bytes of selected location.
    constexpr int locationSelectionInfo = 0;
    rec.put(locationSelectionInfo);

    EmitRecord(data, rec.bytes());

    data.archiveIndex = (data.archiveIndex + 1) % ARCHIVE_SLOTS;
  }

  void ServerDemo::AppendNetworkRecord(ClientData& data, const int sequence, const unsigned char* wireData, const int wireLength)
  {
    // [type 0][sequence][length][message as sent]. The reader takes the sequence as the
    // message's serverMessageSequence, which becomes the snapshot's messageNum that later
    // deltas resolve against, so it has to be the netchan sequence the message is sent
    // with.
    if (wireLength <= 0) return;

    RecordBuilder rec;
    constexpr unsigned char msgType = 0;

    rec.put(msgType);
    rec.put(sequence);
    rec.put(wireLength);
    rec.put(wireData, static_cast<std::size_t>(wireLength));

    EmitRecord(data, rec.bytes());
  }

  void ServerDemo::OnTransmit(Game::client_s* cl, const unsigned char* wireData, const int wireLength)
  {
    if (!cl || !wireData || wireLength <= 0) return;

    // Only real clients that are loading in or active are recorded, never bots.
    if (cl->bIsTestClient) return;
    if (cl->header.state != Game::CS_CLIENTLOADING && cl->header.state != Game::CS_ACTIVE) return;

    const auto clientNum = static_cast<int>(cl - Game::svs_clients);
    if (!ValidClientNum(clientNum)) return;

    // A new map invalidates every gamestate, so every recording and buffer ends together.
    if (HasMapChanged())
    {
      ResetAll(true);
    }

    auto& data = Clients[clientNum];

    // OnClientDisconnected does not see every slot release: SV_FreeClient skips
    // ClientDisconnect while the server is not running, and for a CS_RECONNECTING client
    // left over from an earlier map. A changed lastConnectTime catches the slot being
    // reused anyway.
    if (data.lastConnectTime && *data.lastConnectTime != cl->lastConnectTime)
    {
      ResetClient(clientNum, false);
    }
    data.lastConnectTime = cl->lastConnectTime;

    EnsureClientState(clientNum, cl);

    const auto clientLoading = (cl->header.state == Game::CS_CLIENTLOADING);

    // There is no position worth archiving until the player is in the world.
    if (!clientLoading)
    {
      AppendArchiveRecord(data, cl);
    }

    // Netchan_Transmit stamps the message with the current outgoingSequence (0x46BAAB) and
    // only then increments it, so read before the call-through this is the sequence the
    // message is sent with.
    AppendNetworkRecord(data, cl->header.netchan.outgoingSequence, wireData, wireLength);

    if (!data.fileActive && !data.autoRecordSuppressed && SVDemoAutoRecord.get<bool>())
    {
      // Do not retry a failed start on every message to this client.
      if (!StartRecording(clientNum, true))
      {
        data.autoRecordSuppressed = true;
        Logger::Print("[ServerDemo] Auto-recording disabled for client {} for this connection\n", clientNum);
      }
    }
  }

  bool ServerDemo::SV_Netchan_Transmit_Stub(Game::client_s* client, const void* data, const int length)
  {
    // Record before calling through: SV_Netchan_Transmit encodes the message in place
    // (0x628400) before sending it.
    OnTransmit(client, static_cast<const unsigned char*>(data), length);

    return Utils::Hook::Call<bool(Game::client_s*, const void*, int)>(SV_NETCHAN_TRANSMIT)(client, data, length);
  }

  bool ServerDemo::StartRecording(const int clientNum, const bool automatic)
  {
    if (!ValidClientNum(clientNum)) return false;

    auto& data = Clients[clientNum];
    if (data.fileActive)
    {
      if (!automatic)
      {
        Logger::Print("[ServerDemo] Client {} is already being recorded\n", clientNum);
      }
      return false;
    }

    auto* cl = &Game::svs_clients[clientNum];
    if (cl->header.state < Game::CS_CONNECTED)
    {
      if (!automatic)
      {
        Logger::Print("[ServerDemo] Client {} is not connected\n", clientNum);
      }
      return false;
    }

    if (cl->bIsTestClient)
    {
      if (!automatic)
      {
        Logger::Print("[ServerDemo] Client {} is a bot, not recording\n", clientNum);
      }
      return false;
    }

    // Without the full history since connect, a file would not start at the gamestate.
    if (data.bufferRetired)
    {
      if (!automatic)
      {
        Logger::Print("[ServerDemo] Client {} cannot be recorded - buffered history exceeded sv_demoBufferSize "
          "({} KiB). Raise it, or have the client reconnect\n", clientNum, SVDemoBufferSize.get<int>());
      }
      return false;
    }

    // Nothing has been sent to this slot yet. Auto-recording starts from the first message.
    if (!data.bufferActive)
    {
      Logger::Print("[ServerDemo] Client {} cannot be recorded yet - no buffered data\n", clientNum);
      return false;
    }

    if (automatic)
    {
      CleanupOldAutoDemos();
    }

    auto label = TextRenderer::StripColors(cl->name);
    std::erase_if(label, [](const unsigned char ch) { return !std::isalnum(ch) && ch != '_' && ch != '-'; });
    label.resize(std::min(label.size(), MAX_NAME_CHARS));
    if (label.empty())
    {
      label = std::to_string(clientNum);
    }

    const auto timestamp = static_cast<long long>(std::time(nullptr));
    const auto baseName = std::format("{}{}_{}_{}_{}",
      automatic ? AUTO_PREFIX : "", data.gametype, data.mapname, label, timestamp);
    const auto path = std::format("{}{}.dm_13", DEMO_DIR, baseName);

    const auto handle = Game::FS_FOpenFileWrite(path.data());
    if (!handle)
    {
      Logger::PrintError(Game::CON_CHANNEL_ERROR, "[ServerDemo] Failed to open {} for writing\n", path);
      return false;
    }

    // Seed the file with everything sent since connect, starting at the gamestate.
    if (!data.buffer.empty())
    {
      const auto written = Game::FS_WriteToDemo(data.buffer.data(), static_cast<int>(data.buffer.size()), handle);
      if (written != static_cast<int>(data.buffer.size()))
      {
        Game::FS_FCloseFile(handle);

        Logger::PrintError(Game::CON_CHANNEL_ERROR,
          "[ServerDemo] Failed to seed {} from buffer ({} of {} bytes written) - not recording\n",
          path, written, data.buffer.size());
        return false;
      }
    }

    Logger::Print("[ServerDemo] Started recording client {} ({}) -> {} ({} bytes seeded from buffer)\n",
      clientNum, cl->name, path, data.buffer.size());

    data.demoFile = handle;
    data.fileBaseName = baseName;
    data.fileActive = true;
    data.fileBytes = data.buffer.size();
    data.recordStartMs = Game::Sys_Milliseconds();
    std::time(&data.recordStartTimeStamp);

    return true;
  }

  void ServerDemo::StopRecording(const int clientNum, const bool manual)
  {
    if (!ValidClientNum(clientNum)) return;

    auto& data = Clients[clientNum];
    if (!data.fileActive) return;

    Logger::Print("[ServerDemo] Stopped recording client {}, wrote {}{}.dm_13\n",
      clientNum, DEMO_DIR, data.fileBaseName);

    CloseFile(data);

    // Keep sv_demoAutoRecord from reopening a file on the next message. A later
    // serverrecord still works, seeded from the buffer.
    if (manual)
    {
      data.autoRecordSuppressed = true;
    }
  }

  void ServerDemo::StartAll(const bool automatic)
  {
    for (auto i = 0; i < static_cast<int>(Game::MAX_CLIENTS); ++i)
    {
      if (Game::svs_clients[i].header.state >= Game::CS_CONNECTED)
      {
        StartRecording(i, automatic);
      }
    }
  }

  void ServerDemo::StopAll(const bool manual)
  {
    for (auto i = 0; i < static_cast<int>(Game::MAX_CLIENTS); ++i)
    {
      StopRecording(i, manual);
    }
  }

  void ServerDemo::OnClientDisconnected(const int clientNum)
  {
    ResetClient(clientNum, false);
  }

  void ServerDemo::ServerRecordCommand(const Command::Params* params)
  {
    if (!Dedicated::IsRunning())
    {
      Logger::Print("serverrecord: server is not running\n");
      return;
    }

    if (params->size() != 2)
    {
      Logger::Print("usage: serverrecord <clientnum|-1>\n");
      return;
    }

    const auto arg = std::strtol(params->get(1), nullptr, 10);
    if (arg == -1)
    {
      StartAll(false);
      return;
    }

    if (!ValidClientNum(static_cast<int>(arg)))
    {
      Logger::Print("serverrecord: invalid client num {}\n", arg);
      return;
    }

    StartRecording(static_cast<int>(arg));
  }

  void ServerDemo::ServerStopRecordCommand(const Command::Params* params)
  {
    if (params->size() != 2)
    {
      Logger::Print("usage: serverstoprecord <clientnum|-1>\n");
      return;
    }

    const auto arg = std::strtol(params->get(1), nullptr, 10);
    if (arg == -1)
    {
      StopAll(true);
      return;
    }

    if (!ValidClientNum(static_cast<int>(arg)))
    {
      Logger::Print("serverstoprecord: invalid client num {}\n", arg);
      return;
    }

    StopRecording(static_cast<int>(arg), true);
  }

  ServerDemo::ServerDemo()
  {
    SVDemoAutoRecord = Dvar::Register<bool>("sv_demoAutoRecord", false, Game::DVAR_NONE,
      "Automatically record a server-side demo for every connecting client (dedicated/listen server only)");
    SVDemosKeep = Dvar::Register<int>("sv_demosKeep", 100, 1, 999, Game::DVAR_NONE,
      "How many auto-recorded server demos to keep in total, across all maps, gametypes and players. The oldest "
      "are deleted first");
    SVDemoMaxFileSize = Dvar::Register<int>("sv_demoMaxFileSize", 512, 0, 16384, Game::DVAR_NONE,
      "Maximum MiB for a single server demo. On reaching it the demo is closed cleanly and recording for that "
      "connection stops - a demo cannot be split, it must start at a gamestate. 0 disables the cap");
    SVDemoBufferSize = Dvar::Register<int>("sv_demoBufferSize", 8192, 256, 32768, Game::DVAR_NONE,
      "Per-client limit, in KiB, on the history kept so a recording can start from the client's connect. Memory "
      "use scales with player count: the default allows about 144 MiB across 18 clients, plus headroom while a "
      "buffer grows. Buffering stops for a client once it reaches the limit");

    // Closes the demo as soon as the player leaves. The lastConnectTime check in OnTransmit
    // only notices once the slot is reused, which may never happen.
    Events::OnClientDisconnect(OnClientDisconnected);

    // Every message to every client passes through this call exactly once.
    Utils::Hook(SV_NETCHAN_TRANSMIT_CALL_SITE, SV_Netchan_Transmit_Stub, HOOK_CALL).install()->quick();

    Scheduler::OnGameShutdown([]
    {
      ResetAll(false);
    });

    Command::Add("serverrecord", ServerRecordCommand);
    Command::Add("serverstoprecord", ServerStopRecordCommand);
  }
}
