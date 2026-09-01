#pragma once

namespace Components
{
	// Server-side demo recording.
	//
	// IW4x's existing demo recorder (Theatre.cpp) is entirely client-side: it hooks the
	// client's own demo-write primitives (Game::FS_WriteToDemo, CL_WriteDemoClientArchive)
	// and reads from client-only state (Game::clientConnections, Game::cgArray) that simply
	// doesn't exist when this DLL runs headless as a dedicated server.
	//
	// This module instead taps Events::OnSVSendClientMessage (fired from
	// Voice::SV_SendClientMessages_Stub, which already owns the 0x4519F5 hook point wrapping
	// SV_EndClientSnapshot at 0x4F5300) to capture, once per connected client per server frame,
	// the complete final pre-compression wire message the server is about to send that client.
	// That message is Huffman-compressed and written to a server-owned demo file using the
	// same per-message file framing Theatre::WriteBaseline already uses client-side
	// (byte0 / sequence / compressedSize+4 / byte8=8 / compressed payload chunks).
	//
	// UPDATE: the initial svc_gamestate gap described above has since been closed. Live
	// disassembly found the real, previously-unreversed SV_SendClientGameState (0x625570)
	// and the message builder it calls, FUN_00625270(client_s*, msg_t*) (self-identified via
	// its caller's own debug string "SV_SendClientGameState() for %s\n"). Its call site
	// (0x6256B3) is hooked the same way Voice.cpp hooks the snapshot builder - calling
	// convention confirmed by comparing raw disassembly against that already-proven hook
	// (identical PUSH msg / PUSH client / CALL pattern). The resulting message is cached per
	// client the moment it's sent (independent of whether recording is active yet) and
	// prepended as the demo's first frame once a recording actually starts. This new hook
	// point has NOT been live-tested as of this writing, unlike the per-frame snapshot path
	// which has - treat it as unverified until confirmed against a real server.
	class ServerDemo : public Component
	{
	public:
		ServerDemo();

	private:
		struct DemoInfo
		{
			std::string mapname;
			std::string gametype;
			std::string clientName;
			int length = 0;
			std::time_t timeStamp{};

			[[nodiscard]] nlohmann::json to_json() const;
		};

		struct Session
		{
			bool active = false;
			bool autoRecorded = false;
			int demoFile = 0;
			int messageSequence = 0;
			// Rolling 0-255 index stamped on each type-1 client-archive record. The engine
			// stores archives in a 256-entry table and rejects any index >= 256 outright
			// ("Demo file was corrupt."), and its own writer increments-and-masks this the
			// same way after every record.
			int archiveIndex = 0;
			std::string name; // base filename, no extension, e.g. "dom_mp_carbon_3_timestamp"
			DemoInfo info;
		};

		static constexpr const char* DEMO_DIR = "serverdemos/";
		static constexpr const char* AUTO_PREFIX = "auto_";

		// client_s::name is a 16-byte field, so a sanitised name can never legitimately need
		// more than this; the cap also stops a hostile name bloating the path.
		static constexpr std::size_t MAX_NAME_CHARS = 15;

		static std::array<Session, Game::MAX_CLIENTS> Sessions;

		// Most recently sent gamestate message per client slot, raw uncompressed bytes,
		// captured the instant SV_SendClientGameState fires - independent of whether a
		// recording is active for that client yet. Cleared once consumed by StartRecording,
		// so a stale gamestate from a previous connection can never be attached to a later one.
		static std::array<std::vector<unsigned char>, Game::MAX_CLIENTS> CachedGamestate;

		static Dvar::Var SVDemoAutoRecord;
		static Dvar::Var SVDemosKeep;

		[[nodiscard]] static bool ValidClientNum(int clientNum);

		// Strips colour codes and anything path-unsafe out of a player name so it can be
		// used in a demo filename. Returns empty if nothing usable survives.
		[[nodiscard]] static std::string SanitizeName(const char* name);

		static void StartRecording(int clientNum, bool automatic = false);
		static void StopRecording(int clientNum);
		static void StartAll(bool automatic = false);
		static void StopAll();

		static void CleanupOldAutoDemos();

		// Shared by both the live per-frame path and the cached-gamestate replay path: Huffman
		// compresses data[0..size) and writes it as one framed demo message (the same
		// byte0/sequence/size/byte8/payload layout Theatre::WriteBaseline uses client-side).
		static void WriteRawMessage(int clientNum, const unsigned char* data, int size);

		static void WriteFrame(int clientNum, Game::msg_t* msg);

		// Writes a type-1 "client archive" record: the local player's own position/view for
		// this frame. Without these the demo plays, but the viewpoint has nothing to anchor
		// to - the player renders at a garbage position, snapping only when a snapshot
		// happens to correct it. Emitted once per snapshot message, mirroring the client
		// recorder (which writes one from inside CL_ParseSnapshot).
		static void WriteClientArchive(int clientNum, Game::client_s* cl);

		static void OnServerMessage(Game::client_s* cl, Game::msg_t* msg);

		// Hooks the call site of FUN_00625270 inside SV_SendClientGameState (0x6256B3).
		static void SV_SendClientGameState_Stub(Game::client_s* client, Game::msg_t* msg);
		static void OnGamestateMessage(Game::client_s* cl, Game::msg_t* msg);

		// Closes any demo still open for a slot and starts a fresh one, after a short settle
		// window. Driven from the gamestate hook, which fires both on connect and on every
		// map change - so players who stay across a map change get a new demo instead of
		// silently stopping being recorded.
		static void RestartRecordingDeferred(int clientNum);
		static void OnClientDisconnected(int clientNum);

		static void ServerRecordCommand(const Command::Params* params);
		static void ServerStopRecordCommand(const Command::Params* params);
	};
}
