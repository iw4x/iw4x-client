#include "ServerDemo.hpp"

#include "Dedicated.hpp"
#include "Events.hpp"
#include "FileSystem.hpp"
#include "Scheduler.hpp"

namespace Components
{
	std::array<ServerDemo::Session, Game::MAX_CLIENTS> ServerDemo::Sessions;
	std::array<std::vector<unsigned char>, Game::MAX_CLIENTS> ServerDemo::CachedGamestate;

	Dvar::Var ServerDemo::SVDemoAutoRecord;
	Dvar::Var ServerDemo::SVDemosKeep;

	nlohmann::json ServerDemo::DemoInfo::to_json() const
	{
		std::tm tm{};
		localtime_s(&tm, &timeStamp);
		char dateBuf[64]{};
		asctime_s(dateBuf, sizeof(dateBuf), &tm);

		// Field names/shape intentionally match the sidecar schema already produced by the
		// other titles' demo storage (mapname/gametype/length/date/server/timestamp), so any
		// shared demo-browsing tooling can ingest this without special-casing IW4x.
		return nlohmann::json
		{
			// These five keys are REQUIRED by Theatre::LoadDemos, which reads them with
			// metaObject["key"].get<T>() while only catching nlohmann::json::parse_error.
			// A missing key yields null, and .get<std::string>() on null throws type_error -
			// which is NOT caught, so it escapes as an unhandled C++ exception and crashes the
			// game with 0xE06D7363 the moment the Theatre menu is opened. Do not remove them.
			{ "author", clientName },
			{ "mapname", mapname },
			{ "gametype", gametype },
			{ "length", length },
			{ "timestamp", std::to_string(timeStamp) },

			// Extra keys for parity with the sidecar schema used by the other titles' demo
			// storage, so shared demo-browsing tooling can ingest these unchanged.
			{ "date", dateBuf },
			{ "map", mapname },
			{ "mod", "" },
			{ "revision", "iw4x-serverdemo-v1" },
			{ "server", Dvar::Var("sv_hostname").get<std::string>() },
			{ "recordedClient", clientName },
		};
	}

	bool ServerDemo::ValidClientNum(const int clientNum)
	{
		return clientNum >= 0 && clientNum < static_cast<int>(Game::MAX_CLIENTS);
	}

	std::string ServerDemo::SanitizeName(const char* name)
	{
		// Player names are not safe to drop into a path as-is: they carry IW colour codes
		// ('^' followed by a digit), spaces, and potentially any byte a client felt like
		// sending. Deliberately not using the engine's I_CleanStr here - it rewrites the
		// string in place and we are handed a pointer straight into live client_s state.
		std::string out;

		if (!name)
		{
			return out;
		}

		for (auto i = 0u; name[i] != '\0' && out.size() < MAX_NAME_CHARS; ++i)
		{
			const auto c = static_cast<unsigned char>(name[i]);

			// Skip colour code pairs entirely (^ plus the digit that follows it).
			if (c == '^' && name[i + 1] >= '0' && name[i + 1] <= '9')
			{
				++i;
				continue;
			}

			if (std::isalnum(c))
			{
				out.push_back(static_cast<char>(c));
			}
			else if (c == '_' || c == '-')
			{
				out.push_back(static_cast<char>(c));
			}
			// Everything else (spaces, punctuation, non-ASCII, control bytes) is dropped
			// rather than substituted, to keep filenames predictable.
		}

		return out;
	}

	void ServerDemo::CleanupOldAutoDemos()
	{
		const auto keep = SVDemosKeep.get<int>();
		if (keep <= 0) return;

		std::vector<std::string> files;
		auto demos = FileSystem::GetFileList(DEMO_DIR, "dm_13");

		for (auto& demo : demos)
		{
			if (Utils::String::StartsWith(demo, AUTO_PREFIX))
			{
				files.push_back(demo);
			}
		}

		// Oldest first (timestamp is embedded in the filename and sorts lexicographically
		// the same as numerically for our fixed-width-ish naming), so trimming from the
		// front removes the oldest entries once we're over the keep limit.
		std::ranges::sort(files);

		const auto numDel = static_cast<int>(files.size()) - keep;
		for (auto i = 0; i < numDel; ++i)
		{
			Logger::Print("[ServerDemo] Deleting old auto demo {}\n", files[i]);
			FileSystem::_DeleteFile(DEMO_DIR, files[i]);
			FileSystem::_DeleteFile(DEMO_DIR, std::format("{}.json", files[i]));
		}
	}

	void ServerDemo::StartRecording(const int clientNum, const bool automatic)
	{
		if (!ValidClientNum(clientNum)) return;

		auto& session = Sessions[clientNum];
		if (session.active)
		{
			Logger::Print("[ServerDemo] Client {} is already being recorded\n", clientNum);
			return;
		}

		auto* cl = &Game::svs_clients[clientNum];
		if (cl->header.state < Game::CS_CONNECTED)
		{
			Logger::Print("[ServerDemo] Client {} is not connected\n", clientNum);
			return;
		}

		// Never record bots/test clients. On a bot-filled server this would otherwise spam
		// one demo file per bot every map (they connect at map start, before any human), which
		// is pure noise and would also churn through sv_demosKeep and evict real players' demos.
		// Note: this flag is only reliable once the client has settled - see RestartRecordingDeferred.
		if (cl->bIsTestClient)
		{
			if (!automatic)
			{
				Logger::Print("[ServerDemo] Client {} is a bot, not recording\n", clientNum);
			}
			return;
		}

		if (automatic)
		{
			CleanupOldAutoDemos();
		}

		const auto timestamp = static_cast<long long>(std::time(nullptr));
		const std::string mapname = (*Game::sv_mapname)->current.string;
		const std::string gametype = (*Game::sv_gametype)->current.string;

		// Prefer the player's actual name over their slot number - a slot number is
		// meaningless once the match is over. Fall back to the number only when the name
		// sanitises down to nothing (all colour codes, or non-ASCII only).
		auto label = SanitizeName(cl->name);
		if (label.empty())
		{
			label = std::to_string(clientNum);
		}

		const auto name = std::format("{}{}_{}_{}_{}",
			automatic ? AUTO_PREFIX : "", gametype, mapname, label, timestamp);
		const auto path = std::format("{}{}.dm_13", DEMO_DIR, name);

		const auto handle = Game::FS_FOpenFileWrite(path.data());
		if (!handle)
		{
			Logger::PrintError(Game::CON_CHANNEL_ERROR, "[ServerDemo] Failed to open {} for writing\n", path);
			return;
		}

		session = Session{};
		session.active = true;
		session.autoRecorded = automatic;
		session.demoFile = handle;
		session.messageSequence = 0;
		session.archiveIndex = 0;
		session.name = name;
		session.info.mapname = mapname;
		session.info.gametype = gametype;
		session.info.clientName = cl->name;
		session.info.length = Game::Sys_Milliseconds();
		std::time(&session.info.timeStamp);

		// If we captured this client's gamestate message at connect time (see
		// OnGamestateMessage), write it as message 0 now that `session.active` is true and
		// WriteRawMessage will actually accept it. Consume-once: a stale gamestate from an
		// earlier connection must never attach to a later recording of the same slot.
		auto& gamestate = CachedGamestate[clientNum];
		if (!gamestate.empty())
		{
			const auto gamestateSize = gamestate.size();
			WriteRawMessage(clientNum, gamestate.data(), static_cast<int>(gamestateSize));
			gamestate.clear();
			Logger::Print("[ServerDemo] Prepended cached gamestate for client {} ({} bytes)\n",
				clientNum, gamestateSize);
		}
		else
		{
			Logger::Print("[ServerDemo] No cached gamestate for client {} - recording will start mid-stream\n",
				clientNum);
		}

		// Force the next snapshot sent to this client to be a FULL (non-delta) one. Snapshots
		// are normally delta-compressed against an earlier frame; the first frame we capture
		// would otherwise reference a baseline that was sent before recording began and so is
		// absent from the file, leaving the client unable to reconstruct it. The engine takes
		// the non-delta path whenever deltaMessage < 1 (verified in the snapshot writer:
		// `if (deltaMessage < 1 || state != CS_ACTIVE)` -> full snapshot). This is the
		// server-side counterpart to the client recorder's clc.demowaiting keyframe wait.
		cl->header.deltaMessage = -1;

		Logger::Print("[ServerDemo] Started recording client {} ({}) -> {}\n", clientNum, cl->name, path);
	}

	void ServerDemo::StopRecording(const int clientNum)
	{
		if (!ValidClientNum(clientNum)) return;

		auto& session = Sessions[clientNum];
		if (!session.active) return;

		session.active = false;

		if (session.demoFile)
		{
			Game::FS_FCloseFile(session.demoFile);
			session.demoFile = 0;
		}

		session.info.length = Game::Sys_Milliseconds() - session.info.length;

		FileSystem::FileWriter meta(std::format("{}{}.dm_13.json", DEMO_DIR, session.name));
		meta.write(nlohmann::json(session.info.to_json()).dump());

		Logger::Print("[ServerDemo] Stopped recording client {}, wrote {}{}.dm_13\n",
			clientNum, DEMO_DIR, session.name);

		session = Session{};
	}

	void ServerDemo::StartAll(const bool automatic)
	{
		const auto max = (*Game::sv_maxclients)->current.integer;
		for (auto i = 0; i < max && i < static_cast<int>(Game::MAX_CLIENTS); ++i)
		{
			if (Game::svs_clients[i].header.state >= Game::CS_CONNECTED)
			{
				StartRecording(i, automatic);
			}
		}
	}

	void ServerDemo::StopAll()
	{
		for (auto i = 0; i < static_cast<int>(Game::MAX_CLIENTS); ++i)
		{
			StopRecording(i);
		}
	}

	void ServerDemo::WriteRawMessage(const int clientNum, const unsigned char* data, const int size)
	{
		if (!ValidClientNum(clientNum)) return;

		auto& session = Sessions[clientNum];
		if (!session.active || !session.demoFile) return;
		if (!data || size <= 0) return;

		static unsigned char cmpData[131072];

		if (size >= static_cast<int>(sizeof(cmpData)))
		{
			// Should not happen in practice - engine snapshot/gamestate messages are bounded
			// well under this - but bail rather than overrun our buffer if it ever does.
			Logger::PrintError(Game::CON_CHANNEL_ERROR,
				"[ServerDemo] Message for client {} exceeds compress buffer ({} bytes), skipping\n",
				clientNum, size);
			return;
		}

		// Every message the server builds for a client begins with a 4-byte long: the engine
		// does MSG_Init, then MSG_WriteLong(msg, client->lastClientCommand) before writing any
		// server commands or snapshot/gamestate payload (confirmed by disassembling both
		// SV_SendClientGameState and the per-client send loop). The client reads that same long
		// back as clc.reliableAcknowledge.
		//
		// In the .dm_13 file that long is stored RAW, OUTSIDE the Huffman-compressed body:
		//   [msgType:1][sequence:4][payloadLen:4][reliableAcknowledge:4][huffman(body)]
		// (framing per CL_WriteDemoMessage; the split confirmed by CL_ReadDemoNetworkPacket,
		// which reads the long straight off the uncompressed buffer and range-checks it
		// against clc.reliableSequence - MAX_RELIABLE_COMMANDS before parsing the rest.)
		//
		// This is why Theatre::WriteBaseline writes a bare constant there and compresses a
		// buffer containing no leading long at all. Compressing the WHOLE message and also
		// prepending a dummy long - as this function originally did - left an extra 4 bytes at
		// the head of the decompressed stream, desyncing the parser and dropping the client.
		if (size < static_cast<int>(sizeof(int)))
		{
			return;
		}

		int reliableAcknowledge;
		std::memcpy(&reliableAcknowledge, data, sizeof(int));

		const auto* body = data + sizeof(int);
		const auto bodySize = size - static_cast<int>(sizeof(int));

		const auto compressedSize = Utils::Huffman::Compress(body, cmpData, bodySize, sizeof(cmpData));
		if (compressedSize <= 0) return;

		// Payload length counts the raw reliableAcknowledge long plus the compressed body.
		const auto payloadSize = compressedSize + static_cast<int>(sizeof(int));
		constexpr unsigned char msgType = 0;

		Game::FS_WriteToDemo(&msgType, sizeof(msgType), session.demoFile);
		Game::FS_WriteToDemo(&session.messageSequence, sizeof(int), session.demoFile);
		Game::FS_WriteToDemo(&payloadSize, sizeof(int), session.demoFile);
		Game::FS_WriteToDemo(&reliableAcknowledge, sizeof(int), session.demoFile);

		for (auto i = 0; i < compressedSize; i += 1024)
		{
			const auto chunk = std::min(compressedSize - i, 1024);
			Game::FS_WriteToDemo(&cmpData[i], chunk, session.demoFile);
		}

		++session.messageSequence;
	}

	void ServerDemo::WriteFrame(const int clientNum, Game::msg_t* msg)
	{
		if (!msg || !msg->data || msg->cursize <= 0) return;
		WriteRawMessage(clientNum, msg->data, msg->cursize);
	}

	void ServerDemo::WriteClientArchive(const int clientNum, Game::client_s* cl)
	{
		if (!ValidClientNum(clientNum) || !cl) return;

		auto& session = Sessions[clientNum];
		if (!session.active || !session.demoFile) return;

		// Server-side source of truth for this player's state. Not populated until the client
		// has actually spawned in, so skip until then rather than archiving a null/garbage view.
		if (!cl->gentity || !cl->gentity->client) return;

		const auto* ps = &cl->gentity->client->ps;

		// Layout is fixed by the engine's own reader (type-1 branch of the demo dispatcher):
		//   index(4) origin(12) velocity(12) movementDir(4) bobCycle(4) commandTime(4)
		//   viewangles(12) locationSelectionInfo(4)
		// Note the field order on disk is NOT struct order - movementDir and bobCycle are
		// written before the first 4 bytes of playerState_s (commandTime). Unlike type-0
		// records this one is raw: no length prefix and no Huffman compression.
		constexpr unsigned char msgType = 1;
		Game::FS_WriteToDemo(&msgType, sizeof(msgType), session.demoFile);

		Game::FS_WriteToDemo(&session.archiveIndex, sizeof(int), session.demoFile);
		Game::FS_WriteToDemo(ps->origin, sizeof(float[3]), session.demoFile);
		Game::FS_WriteToDemo(ps->velocity, sizeof(float[3]), session.demoFile);
		Game::FS_WriteToDemo(&ps->movementDir, sizeof(int), session.demoFile);
		Game::FS_WriteToDemo(&ps->bobCycle, sizeof(int), session.demoFile);
		Game::FS_WriteToDemo(&ps->commandTime, sizeof(int), session.demoFile);
		Game::FS_WriteToDemo(ps->viewangles, sizeof(float[3]), session.demoFile);

		// 0 = no location selector active. Non-zero would oblige us to write 8 more bytes of
		// selectedLocation, and IW4x deliberately disables this field client-side anyway
		// (Theatre::CL_WriteDemoClientArchive_Hk, "Fix issue with locationSelectionInfo").
		constexpr int locationSelectionInfo = 0;
		Game::FS_WriteToDemo(&locationSelectionInfo, sizeof(int), session.demoFile);

		session.archiveIndex = (session.archiveIndex + 1) & 0xFF;
	}

	void ServerDemo::OnServerMessage(Game::client_s* cl, Game::msg_t* msg)
	{
		if (!cl) return;

		const auto clientNum = static_cast<int>(cl - Game::svs_clients);
		WriteFrame(clientNum, msg);

		// Pair every network message with the archived viewpoint for that same frame.
		WriteClientArchive(clientNum, cl);
	}

	void ServerDemo::SV_SendClientGameState_Stub(Game::client_s* client, Game::msg_t* msg)
	{
		// FUN_00625270: builds the svc_gamestate message (configstring dump + baseline framing)
		// into msg. Real engine name unconfirmed by symbol, but its only caller (0x625570)
		// self-identifies via its own debug format string: "SV_SendClientGameState() for %s\n".
		Utils::Hook::Call<void(Game::client_s*, Game::msg_t*)>(0x625270)(client, msg);

		OnGamestateMessage(client, msg);
	}

	void ServerDemo::OnGamestateMessage(Game::client_s* cl, Game::msg_t* msg)
	{
		if (!cl || !msg || !msg->data || msg->cursize <= 0) return;

		const auto clientNum = static_cast<int>(cl - Game::svs_clients);
		if (!ValidClientNum(clientNum)) return;

		// Close any demo still open for this slot IMMEDIATELY, before caching the new
		// gamestate. A gamestate means the client is (re)entering the world, so the previous
		// recording is finished. Doing this here rather than in the deferred restart matters:
		// the deferred start runs a second later, and in that window snapshots for the NEW
		// map would otherwise be appended to the PREVIOUS map's demo.
		StopRecording(clientNum);

		// Cache unconditionally, whether or not a recording is active yet - auto-record's
		// settle delay means recording usually starts a moment AFTER gamestate has already
		// gone out, so this is the only way to still capture it.
		CachedGamestate[clientNum].assign(msg->data, msg->data + msg->cursize);

		// A gamestate is sent to a client whenever it (re)enters the world - on connect AND
		// on every map change / map_restart. That makes this the right place to (re)start a
		// recording, and it fixes a real gap: previously recording only began on connect, so
		// a player who stayed across a map change was never recorded again. Their old demo
		// was closed at map end and nothing replaced it.
		//
		// Rotate here: close any demo still open for this slot, then start a fresh one for
		// the new map. Deferred for the same reason as the connect path - client_s::bIsTestClient
		// and the player's name are not populated yet at this instant.
		RestartRecordingDeferred(clientNum);
	}

	// Closes any active recording for a slot and starts a new one, after a short settle
	// window. Safe to call for a client that is not being recorded.
	void ServerDemo::RestartRecordingDeferred(const int clientNum)
	{
		if (!Dedicated::IsRunning()) return;
		if (!SVDemoAutoRecord.get<bool>()) return;
		if (!ValidClientNum(clientNum)) return;

		Scheduler::Once([clientNum]
		{
			if (!Dedicated::IsRunning()) return;
			if (!SVDemoAutoRecord.get<bool>()) return;

			const auto* cl = &Game::svs_clients[clientNum];

			// Client may have dropped during the settle window.
			if (cl->header.state < Game::CS_CONNECTED) return;

			// The previous demo was already closed synchronously when the gamestate arrived,
			// so this only ever opens a new one. StartRecording re-checks the bot flag, which
			// is trustworthy by now.
			StartRecording(clientNum, true);
		}, Scheduler::Pipeline::SERVER, 1s);
	}

	void ServerDemo::OnClientDisconnected(const int clientNum)
	{
		StopRecording(clientNum);

		if (ValidClientNum(clientNum))
		{
			CachedGamestate[clientNum].clear();
		}
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
			StopAll();
			return;
		}

		if (!ValidClientNum(static_cast<int>(arg)))
		{
			Logger::Print("serverstoprecord: invalid client num {}\n", arg);
			return;
		}

		StopRecording(static_cast<int>(arg));
	}

	ServerDemo::ServerDemo()
	{
		SVDemoAutoRecord = Dvar::Register<bool>("sv_demoAutoRecord", false, Game::DVAR_NONE,
			"Automatically record a server-side demo for every connecting client (dedicated/listen server only)");
		SVDemosKeep = Dvar::Register<int>("sv_demosKeep", 100, 1, 999, Game::DVAR_NONE,
			"How many auto-recorded server demos to keep per rotation");

		Events::OnSVSendClientMessage(OnServerMessage);
		Events::OnClientDisconnect(OnClientDisconnected);

		// Hooks the call to FUN_00625270 inside SV_SendClientGameState (0x625570) - the
		// engine's own gamestate message builder. Confirmed via disassembly to be untouched by
		// any other module (single call site, no other hook installed here), and its calling
		// convention (client, msg) was cross-checked byte-for-byte against Voice.cpp's already-
		// proven hook at 0x4519F5 before adding this. Unlike that hook, this one has not yet
		// been confirmed against a live server - see ServerDemo.hpp.
		Utils::Hook(0x6256B3, SV_SendClientGameState_Stub, HOOK_CALL).install()->quick();

		// Belt-and-braces: make sure nothing bleeds across a map change/server shutdown.
		Scheduler::OnGameShutdown(StopAll);

		Command::Add("serverrecord", ServerRecordCommand);
		Command::Add("serverstoprecord", ServerStopRecordCommand);
	}
}
