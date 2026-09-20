#pragma once

namespace Components
{
	// Server-side demo recording.
	//
	// REWRITE. The original design hooked two different points (SV_EndClientSnapshot for
	// per-frame snapshots, SV_SendClientGameState for the initial gamestate) and cached a
	// single gamestate message at connect time to prepend whenever recording actually started.
	// That had three real problems, all raised in PR review:
	//
	//   1. The cached gamestate goes stale the moment anything in it changes (entity
	//      positions, scores, objective state) - "started recording after a couple of
	//      minutes" would replay a frame-zero gamestate against live current-state traffic.
	//   2. Recording start forced client_s::header.deltaMessage = -1 to get a full snapshot,
	//      but a snapshot already in flight the instant recording starts can still legitimately
	//      delta against something older than that forced keyframe and missing from the file -
	//      which does not corrupt "a few frames", it corrupts everything from that point on,
	//      since every later snapshot deltas against it. This is a known bug in CoD4x's own
	//      server demo implementation.
	//   3. The client archive (viewpoint) record was populated by hand-walking
	//      client->gentity->client->ps from a SEPARATE hook, timed independently of the
	//      network capture. On IW5 this exact pattern (unguarded pointer chain, wrong timing)
	//      hung the server outright.
	//
	// This version follows the architecture used by AlterWare's iw6-mod server demo recorder
	// (src/client/component/demo_sv_recording.cpp): ONE hook, on the single function every
	// outgoing message to every client passes through exactly once - SV_Netchan_Transmit,
	// confirmed here via Ghidra as FUN_0047CB60(client_s*, const void* data, int length),
	// called from exactly one place (SV_SendMessageToClient, 0x48FE90) at call site 0x48FF67.
	// It receives the FINAL, already-Huffman-compressed bytes, for BOTH loading-state
	// (gamestate/reliable commands) and active-state (snapshot) traffic - so gamestate is just
	// another message flowing through the same capture point, not a separately cached, special
	// case that can go stale.
	//
	// Each client gets a continuously-growing in-memory buffer, alive from the moment they
	// start receiving messages (loading or active) until a map change, a reconnect, or server
	// shutdown clears it. Every hook call appends one record to that buffer AND, if a file is
	// currently open for that client, to the file too. Starting a recording - manually via
	// serverrecord, or automatically - just means opening a file and dumping whatever is
	// already in the buffer as its initial content:
	//
	//   - Gamestate freshness (issue 1) is moot: there is no cached frame to go stale. What
	//     gets written is the literal, real byte stream the engine sent, in order, from the
	//     moment the client connected.
	//   - The corrupt-delta risk (issue 2) is architecturally gone: there is no synthetic
	//     keyframe to force, because the buffer already contains every message since connect,
	//     including the real non-delta gamestate. The delta chain is always complete.
	//   - The archive record (issue 3) is now derived synchronously inside the SAME hook call
	//     that captures the network data, from the same client_s the engine just used to build
	//     that exact message - not a separately-timed pass with its own race window.
	//
	// One deliberate divergence from iw6-mod: IW6's version pairs this architecture with a
	// custom container format and its OWN client-side demo parser (CL_ParseServerMessage fed
	// from re-extracted network_data blobs). IW4x does not - it deliberately keeps writing the
	// real, stock .dm_13 format so the untouched retail demo player (and IW4x's own
	// Theatre.cpp, which already plays these back) can read the result with no client changes.
	// That is also why the archive/client-viewpoint record still exists here at all: the STOCK
	// demo player's theater/camera code reads it as a dedicated data source for the local
	// viewpoint, independent of and at different timing to generic entity-snapshot parsing.
	// The player's own origin/velocity/angles ARE already present inside the network_data
	// blob's playerState - but only as Huffman-compressed, delta-encoded bytes the stock reader
	// was never built to pull a standalone camera position out of; the archive record is how
	// the format is actually specified to carry that, and Theatre.cpp's own client-side
	// recorder writes the identical record for the identical reason.
	class ServerDemo : public Component
	{
	public:
		ServerDemo();

	private:
		// Everything needed to (re)build a demo for one client slot, alive independently of
		// whether a file is currently open. Reset only on map change/restart, reconnect, or
		// shutdown.
		//
		// The buffer holds every record sent to this client since they connected, so that a
		// recording started LATER can be seeded with the traffic it missed. It keeps filling
		// while a file is open, deliberately: that is what lets a serverstoprecord followed by
		// a later serverrecord produce a second COMPLETE demo for the same connection rather
		// than one starting mid-delta-chain. iw6-mod made the same call, and for the same
		// reason (anomaly/iw6-mod 7cd5359: "keep buffering data even when recording a demo so
		// subsequent demos for the same client can be recorded using the entire buffer").
		// What bounds it is the sv_demoBufferSize cap, on hitting which the buffer is retired
		// whole - see EmitRecord.
		struct ClientData
		{
			bool identityValid = false;  // mapname/gametype/clientName captured
			bool bufferActive = false;   // buffer is accumulating
			bool bufferRetired = false;  // hit the cap; stopped and freed, never re-armed
			bool fileActive = false;     // currently recording to disk
			// serverstoprecord was issued for this connection. Blocks the sv_demoAutoRecord
			// path from immediately opening a new file on the very next transmit, which is
			// what previously made the command look like it did nothing. It does NOT block a
			// deliberate serverrecord: the buffer still holds the whole connection, so an
			// admin asking again gets a complete demo, not a fragment.
			bool autoRecordSuppressed = false;

			// Message/archive-index counters. These run for the lifetime of the BUFFER (reset
			// only on map change/reconnect), not the lifetime of any one file: with
			// buffer-everything, "start recording" mid-match means "dump what's already
			// buffered", so the sequence numbering already in that buffer has to stay valid
			// rather than restart at 0 out from under it.
			int messageSequence = 0;
			int archiveIndex = 0; // rolls over at 256 - see WriteArchiveRecord.

			std::optional<int> lastConnectTime; // client_s::lastConnectTime, reconnect detector

			std::vector<unsigned char> buffer;
			int demoFile = 0;
			std::size_t fileBytes = 0; // bytes written to the open file, for sv_demoMaxFileSize
			std::string fileBaseName; // base filename of the currently-open file, no extension

			std::string mapname;
			std::string gametype;
			std::string clientName;
			std::time_t recordStartTimeStamp{};
			int recordStartMs = 0;
		};

		// The reader indexes clientActive_t::clientArchive, so the wrap point is that array's
		// own length rather than a literal 256. std::extent_v (not std::size, which needs an
		// object) is how a non-static member array's bound is read without one.
		static constexpr auto ARCHIVE_SLOTS = std::extent_v<decltype(Game::clientActive_t::clientArchive)>;

		// Reserve up front to minimise further (re)allocations. Not conditional on
		// sv_demoAutoRecord: buffering continues either way, so the old "8 KiB if
		// auto-recording" reserve described a behaviour the code never had. Same value and
		// same reasoning as iw6-mod after anomaly/iw6-mod@534e9c0. What bounds the buffer is
		// sv_demoBufferSize, not this.
		static constexpr std::size_t INITIAL_BUFFER_BYTES = 1024 * 1024;

		static constexpr const char* DEMO_DIR = "serverdemos/";
		static constexpr const char* AUTO_PREFIX = "auto_";
		static constexpr std::size_t MAX_NAME_CHARS = 15;

		// Real signature confirmed via Ghidra decompilation of FUN_0047CB60: cdecl,
		// (client_s*, const void* data, int length) -> bool (success).
		static constexpr std::uintptr_t SV_NETCHAN_TRANSMIT = 0x47CB60;
		// The one and only call site (inside SV_SendMessageToClient, 0x48FE90) - confirmed via
		// Ghidra xref: exactly one caller of 0x47CB60 exists in the whole binary.
		static constexpr std::uintptr_t SV_NETCHAN_TRANSMIT_CALL_SITE = 0x48FF67;

		static std::array<ClientData, Game::MAX_CLIENTS> Clients;

		// Last-seen value of the engine's own serverId, used to detect a map change OR a
		// fast_restart so every client's buffer and any open file can be closed and cleared
		// together. Comparing sv_mapname/sv_gametype instead is not sufficient: a
		// fast_restart keeps both strings identical while the engine still hands every client
		// a fresh gamestate, which would land mid-file and break the delta chain from there
		// on. See CheckServerRestart for the engine evidence.
		static int LastServerId;
		static bool HaveServerId;

		static Dvar::Var SVDemoAutoRecord;
		static Dvar::Var SVDemosKeep;
		static Dvar::Var SVDemoBufferSize;
		static Dvar::Var SVDemoMaxFileSize;

		[[nodiscard]] static bool ValidClientNum(int clientNum);
		[[nodiscard]] static std::string SanitizeName(const char* name);

		// Returns true if the server started a new map or restarted the current one since the
		// last call. Cheap enough to call on every transmit.
		[[nodiscard]] static bool CheckServerRestart();

		// Per-client cap on buffered bytes, from sv_demoBufferSize (KiB).
		[[nodiscard]] static std::size_t MaxBufferBytes();

		// Cap on one demo file, from sv_demoMaxFileSize (MiB). 0 disables the cap.
		[[nodiscard]] static std::size_t MaxFileBytes();

		// Writes the sidecar, closes the handle and clears the file state. The single place
		// a demo file is ever closed, so every exit path writes a sidecar exactly once.
		static void CloseFile(ClientData& data);

		// Full reset for one slot: closes any open file (writing its sidecar first), clears
		// the buffer, resets every counter. Used for map change, reconnect, and shutdown -
		// never for a plain serverrecord/serverstoprecord toggle.
		static void ResetClient(int clientNum, bool wasMapChange);
		static void ResetAll(bool wasMapChange);

		// Captures the map/gametype/name identity for this connection, and arms the buffer if
		// it has not been retired. Called on every transmit; does work only when needed.
		static void EnsureClientState(int clientNum, const Game::client_s* cl);

		// Stops the buffer accumulating and frees its memory, keeping the connection's
		// identity and counters. Called only on sv_demoBufferSize overflow.
		static void RetireBuffer(ClientData& data);

		// Appends one archive (viewpoint) record for the recorded player's own current state,
		// read directly from the same client_s the engine just used to build the message this
		// hook is currently processing - not a separately-timed pass.
		// Appends one already-built record to the buffer (if active) and to the open file (if
		// active) - the single place that decides where a record's bytes actually go, so the
		// buffer and any currently-open file can never drift out of sync with each other.
		static void EmitRecord(ClientData& data, const std::vector<unsigned char>& record);

		static void AppendArchiveRecord(ClientData& data, const Game::client_s* cl);

		// Appends one network-data record: the exact bytes SV_Netchan_Transmit is about to
		// send, verbatim. No re-compression, no re-derivation - these are already the final,
		// Huffman-compressed bytes the engine itself produced.
		static void AppendNetworkRecord(ClientData& data, const unsigned char* wireData, int wireLength);

		// The hook body. Runs the map-rotation check, per-client buffer bookkeeping, the
		// auto-record decision, and the actual record appends, for every message to every
		// loading-or-active client.
		static void OnTransmit(Game::client_s* cl, const unsigned char* wireData, int wireLength);
		static bool SV_Netchan_Transmit_Stub(Game::client_s* client, const void* data, int length);

		static void StartRecording(int clientNum, bool automatic = false);
		// Closes the file. `manual` (a serverstoprecord, not a map change or shutdown) also
		// latches autoRecordSuppressed for the rest of the connection.
		static void StopRecording(int clientNum, bool manual);
		static void StartAll(bool automatic = false);
		static void StopAll(bool manual);

		static void CleanupOldAutoDemos();
		static void WriteSidecar(const ClientData& data, const std::string& baseName);

		static void OnClientDisconnected(int clientNum);

		static void ServerRecordCommand(const Command::Params* params);
		static void ServerStopRecordCommand(const Command::Params* params);
	};
}
