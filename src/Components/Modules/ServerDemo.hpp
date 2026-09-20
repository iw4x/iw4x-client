#pragma once

namespace Components
{
  // Server-side demo recording.
  //
  // Writes one stock .dm_13 per client, so the retail demo player and Theatre can play the
  // result back with no client changes.
  //
  // Every message to a client passes through SV_Netchan_Transmit exactly once, already
  // Huffman-compressed - the gamestate, reliable commands and every snapshot. Hooking that
  // one call gives the exact byte stream the client received, in order. Each client's stream
  // is buffered from connect, so a recording started mid-match is seeded from the start of
  // the connection: the file begins at the real gamestate and every later delta resolves
  // against a message that is also in the file.
  //
  // Alongside each network record goes a client archive record (the recorded player's
  // origin, velocity and view angles). The stock demo player takes the camera from these
  // records, not from the playerState inside the snapshots. Without them, playback spams
  // "Couldn't find exact match for servertime" and the view stays fixed at the origin.
  class ServerDemo : public Component
  {
  public:
    ServerDemo();

  private:
    // Per-slot state. Reset on map change, reconnect, disconnect and shutdown.
    //
    // The buffer holds every record sent to this client since they connected. It keeps
    // filling while a file is open, so that stopping a recording and starting another later
    // in the same connection still produces a complete demo. It is bounded by
    // sv_demoBufferSize; once that is exceeded the buffer is dropped for the rest of the
    // connection, since a demo cannot be seeded from a partial history.
    struct ClientData
    {
      bool identityValid = false; // mapname/gametype/clientName captured
      bool bufferActive = false;  // buffer is accumulating
      bool bufferRetired = false; // exceeded sv_demoBufferSize, dropped for this connection
      bool fileActive = false;    // currently recording to disk

      // Stops sv_demoAutoRecord from opening a file for this connection. Set by
      // serverstoprecord, and when an automatic recording fails or has to be closed early.
      // A manual serverrecord is not affected.
      bool autoRecordSuppressed = false;

      // Index into the reader's clientActive_t::clientArchive ring. Runs for the life of
      // the buffer, not of any one file, because a file is seeded from the buffer.
      int archiveIndex = 0;

      std::optional<int> lastConnectTime; // client_s::lastConnectTime, detects slot reuse

      std::vector<unsigned char> buffer;
      int demoFile = 0;
      std::size_t fileBytes = 0;  // bytes written to the open file
      std::string fileBaseName;   // open file's name, no extension

      std::string mapname;
      std::string gametype;
      std::string clientName;
      std::time_t recordStartTimeStamp{};
      int recordStartMs = 0;
    };

    // Wrap point for archiveIndex: the length of the reader's archive ring.
    static constexpr auto ARCHIVE_SLOTS = std::extent_v<decltype(Game::clientActive_t::clientArchive)>;

    // Initial reservation, to avoid a run of reallocations early in a connection.
    static constexpr std::size_t INITIAL_BUFFER_BYTES = 1024 * 1024;

    static constexpr const char* DEMO_DIR = "serverdemos/";
    static constexpr const char* AUTO_PREFIX = "auto_";
    static constexpr std::size_t MAX_NAME_CHARS = 15;

    // bool SV_Netchan_Transmit(client_s*, const void* data, int length), and its only caller
    // (inside SV_SendMessageToClient).
    static constexpr std::uintptr_t SV_NETCHAN_TRANSMIT = 0x47CB60;
    static constexpr std::uintptr_t SV_NETCHAN_TRANSMIT_CALL_SITE = 0x48FF67;

    static std::array<ClientData, Game::MAX_CLIENTS> Clients;

    // Last seen sv_serverId, for detecting a map change. See HasMapChanged.
    static int LastServerId;
    static bool HaveServerId;

    static Dvar::Var SVDemoAutoRecord;
    static Dvar::Var SVDemosKeep;
    static Dvar::Var SVDemoBufferSize;
    static Dvar::Var SVDemoMaxFileSize;

    [[nodiscard]] static bool ValidClientNum(int clientNum);

    // True if a new map has loaded since the last call. sv_serverId counts maps in its high
    // nibble and restarts in its low nibble, and clients are only sent a new gamestate when
    // the high nibble changes (SV_ExecuteClientMessage, 0x414DFB). After a fast_restart the
    // existing gamestate stays valid, so recordings continue across it.
    [[nodiscard]] static bool HasMapChanged();

    [[nodiscard]] static std::size_t MaxBufferBytes(); // sv_demoBufferSize, in bytes
    [[nodiscard]] static std::size_t MaxFileBytes();   // sv_demoMaxFileSize, in bytes

    // Writes the metadata and closes the file. Every path that ends a recording goes
    // through here.
    static void CloseFile(ClientData& data);

    // Closes any open file and clears all state for the slot.
    static void ResetClient(int clientNum, bool wasMapChange);
    static void ResetAll(bool wasMapChange);

    // Captures the connection's map/gametype/name and arms the buffer unless it was
    // retired.
    static void EnsureClientState(int clientNum, const Game::client_s* cl);

    // Stops buffering for the rest of the connection and frees the memory.
    static void RetireBuffer(ClientData& data);

    // Appends a record to the buffer (if active) and the open file (if any), keeping the
    // two byte-identical.
    static void EmitRecord(ClientData& data, const std::vector<unsigned char>& record);

    static void AppendArchiveRecord(ClientData& data, const Game::client_s* cl);
    static void AppendNetworkRecord(ClientData& data, int sequence, const unsigned char* wireData, int wireLength);

    // Records one message to a loading or active client.
    static void OnTransmit(Game::client_s* cl, const unsigned char* wireData, int wireLength);
    static bool SV_Netchan_Transmit_Stub(Game::client_s* client, const void* data, int length);

    // Returns true if a file was opened.
    static bool StartRecording(int clientNum, bool automatic = false);
    // `manual` is a serverstoprecord; it also suppresses auto-recording for the connection.
    static void StopRecording(int clientNum, bool manual);
    static void StartAll(bool automatic = false);
    static void StopAll(bool manual);

    static void CleanupOldAutoDemos();
    static void WriteMetadata(const ClientData& data, const std::string& baseName);

    static void OnClientDisconnected(int clientNum);

    static void ServerRecordCommand(const Command::Params* params);
    static void ServerStopRecordCommand(const Command::Params* params);
  };
}
