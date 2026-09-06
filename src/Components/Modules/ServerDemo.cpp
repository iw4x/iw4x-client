#include "ServerDemo.hpp"

#include "Dedicated.hpp"
#include "Events.hpp"
#include "FileSystem.hpp"
#include "Scheduler.hpp"

namespace Components
{
	std::array<ServerDemo::ClientData, Game::MAX_CLIENTS> ServerDemo::Clients;

	std::string ServerDemo::LastMapname;
	std::string ServerDemo::LastGametype;
	bool ServerDemo::HaveMapIdentity = false;

	Dvar::Var ServerDemo::SVDemoAutoRecord;
	Dvar::Var ServerDemo::SVDemosKeep;

	bool ServerDemo::ValidClientNum(const int clientNum)
	{
		return clientNum >= 0 && clientNum < static_cast<int>(Game::MAX_CLIENTS);
	}

	std::string ServerDemo::SanitizeName(const char* name)
	{
		// Player names carry IW colour codes ('^' + digit), spaces, and potentially any byte a
		// client felt like sending - none of which belong in a path. Deliberately not using the
		// engine's I_CleanStr here - it rewrites the string in place and we are handed a pointer
		// straight into live client_s state.
		std::string out;
		if (!name) return out;

		for (auto i = 0u; name[i] != '\0' && out.size() < MAX_NAME_CHARS; ++i)
		{
			const auto c = static_cast<unsigned char>(name[i]);

			if (c == '^' && name[i + 1] >= '0' && name[i + 1] <= '9')
			{
				++i;
				continue;
			}

			if (std::isalnum(c) || c == '_' || c == '-')
			{
				out.push_back(static_cast<char>(c));
			}
		}

		return out;
	}

	bool ServerDemo::CheckMapRotation()
	{
		const std::string mapname = (*Game::sv_mapname)->current.string;
		const std::string gametype = (*Game::sv_gametype)->current.string;

		if (!HaveMapIdentity)
		{
			LastMapname = mapname;
			LastGametype = gametype;
			HaveMapIdentity = true;
			return false;
		}

		if (mapname != LastMapname || gametype != LastGametype)
		{
			LastMapname = mapname;
			LastGametype = gametype;
			return true;
		}

		return false;
	}

	void ServerDemo::WriteSidecar(const ClientData& data, const std::string& baseName)
	{
		std::tm tm{};
		localtime_s(&tm, &data.recordStartTimeStamp);
		char dateBuf[64]{};
		asctime_s(dateBuf, sizeof(dateBuf), &tm);

		const auto lengthMs = Game::Sys_Milliseconds() - data.recordStartMs;

		// These five keys are REQUIRED by Theatre::LoadDemos, which reads them with
		// metaObject["key"].get<T>() while only catching nlohmann::json::parse_error. A
		// missing key yields null, and .get<std::string>() on null throws type_error - which
		// is NOT caught, so it escapes as an unhandled C++ exception and crashes the game the
		// moment the Theatre menu is opened. Do not remove them.
		// "author" drives Theatre's ui_demo_author (the theater menu's byline) - for a
		// server-recorded demo that should read as "recorded by this server", not the name of
		// whichever client got recorded, so it's the server's own hostname. The actual client
		// is still preserved separately below, as "recordedClient".
		const auto serverName = Dvar::Var("sv_hostname").get<std::string>();

		const nlohmann::json j
		{
			{ "author", serverName },
			{ "mapname", data.mapname },
			{ "gametype", data.gametype },
			{ "length", lengthMs },
			{ "timestamp", std::to_string(data.recordStartTimeStamp) },

			// Extra keys for parity with the sidecar schema used by the other titles' demo
			// storage, so shared demo-browsing tooling can ingest these unchanged.
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

		std::ranges::sort(files);

		const auto numDel = static_cast<int>(files.size()) - keep;
		for (auto i = 0; i < numDel; ++i)
		{
			Logger::Print("[ServerDemo] Deleting old auto demo {}\n", files[i]);
			FileSystem::_DeleteFile(DEMO_DIR, files[i]);
			FileSystem::_DeleteFile(DEMO_DIR, std::format("{}.json", files[i]));
		}
	}

	void ServerDemo::ResetClient(const int clientNum, const bool wasMapChange)
	{
		if (!ValidClientNum(clientNum)) return;

		auto& data = Clients[clientNum];

		if (data.fileActive)
		{
			WriteSidecar(data, data.fileBaseName);
			Game::FS_FCloseFile(data.demoFile);

			Logger::Print("[ServerDemo] {} client {} ({}), wrote {}{}.dm_13\n",
				wasMapChange ? "Rotated" : "Closed", clientNum, data.clientName, DEMO_DIR, data.fileBaseName);
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

	void ServerDemo::EnsureBufferHeader(const int clientNum, const Game::client_s* cl)
	{
		auto& data = Clients[clientNum];
		if (data.bufferActive) return;

		// Reserve up front: auto-record almost immediately opens a file and starts writing
		// straight to disk, so the buffer never needs to hold much; a manual/admin recording
		// might be started minutes into a match, so the buffer needs to be ready to hold a
		// full match's worth of every message sent to this client since they connected.
		const auto autorecord = SVDemoAutoRecord.get<bool>();
		data.buffer.reserve(autorecord ? 8192 : (512 * 1024));

		data.mapname = (*Game::sv_mapname)->current.string;
		data.gametype = (*Game::sv_gametype)->current.string;
		data.clientName = cl->name;

		data.bufferActive = true;
	}

	namespace
	{
		// Shared little record-builder. Every record (archive or network) is built ONCE into
		// a scratch byte vector, then handed to EmitRecord below to go wherever it needs to -
		// this is what keeps the in-memory buffer and any currently-open file byte-for-byte
		// identical, instead of two independent write call chains that could drift apart.
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

	// Appends one already-built record to the buffer (if active) and to the open file (if
	// active) - the single place that decides where a record's bytes actually go.
	void ServerDemo::EmitRecord(ClientData& data, const std::vector<unsigned char>& record)
	{
		if (record.empty()) return;

		if (data.bufferActive)
		{
			data.buffer.insert(data.buffer.end(), record.begin(), record.end());
		}

		if (data.fileActive)
		{
			Game::FS_WriteToDemo(record.data(), static_cast<int>(record.size()), data.demoFile);
		}
	}

	void ServerDemo::AppendArchiveRecord(ClientData& data, const Game::client_s* cl)
	{
		// Not populated until the client has actually spawned in - skip rather than archive a
		// null/garbage view. Read directly from the SAME client_s the engine just used to
		// build the message this hook call is processing, so there is no separate timing
		// window in which the pointer chain could be stale or the client could have dropped.
		if (!cl->gentity || !cl->gentity->client) return;

		const auto* ps = &cl->gentity->client->ps;

		// Layout is fixed by the engine's own demo reader (type-1 branch of the dispatcher),
		// confirmed by hex-diffing against a real client-recorded .dm_13:
		//   index(4) origin(12) velocity(12) movementDir(4) bobCycle(4) commandTime(4)
		//   viewangles(12) locationSelectionInfo(4)
		// Field order on disk is NOT struct order. Unlike network records this one is raw: no
		// length prefix, no compression.
		RecordBuilder rec;

		constexpr unsigned char msgType = 1;
		rec.put(msgType);
		rec.put(data.archiveIndex);
		rec.put(ps->origin, sizeof(float[3]));
		rec.put(ps->velocity, sizeof(float[3]));
		rec.put(ps->movementDir);
		rec.put(ps->bobCycle);
		rec.put(ps->commandTime);
		rec.put(ps->viewangles, sizeof(float[3]));

		// 0 = no location selector active. Non-zero would oblige 8 more bytes of
		// selectedLocation, and IW4x deliberately disables this field client-side anyway
		// (Theatre::CL_WriteDemoClientArchive_Hk, "Fix issue with locationSelectionInfo").
		constexpr int locationSelectionInfo = 0;
		rec.put(locationSelectionInfo);

		EmitRecord(data, rec.bytes());

		// Rolls over at 256: the engine's own archive table has exactly that many slots and
		// rejects anything else as a corrupt demo.
		data.archiveIndex = (data.archiveIndex + 1) & 0xFF;
	}

	void ServerDemo::AppendNetworkRecord(ClientData& data, const unsigned char* wireData, const int wireLength)
	{
		// wireData is EXACTLY what SV_Netchan_Transmit is about to hand to the network layer:
		// [outgoingSequence:4 raw, netchan transport framing][huffman-compressed body].
		//
		// A previous version of this comment claimed, based on Theatre::WriteBaseline, that
		// this leading 4 bytes should be DROPPED and replaced with a literal constant 8 - that
		// was wrong, and traced back to trusting the wrong reference. Ground truth is the
		// native reader itself (FUN_005a9ba0/CL_ReadDemoNetworkPacket, decompiled directly) and
		// the native WRITER instruction trace around the RecordGamestateStub hook site
		// (0x5A85D2): both agree a type-0 record is exactly [type:1][seq:4][length:4]
		// [length bytes of body] - THREE header fields, not four. WriteBaseline writes FOUR
		// (seq, compressedSize+4, then a spurious extra literal-8 field) - it does not match
		// what either the real reader or the real writer actually do, so it was never a valid
		// reference for this record's shape and the fix built on it just traded one corruption
		// for another. The body is the wire capture verbatim, unmodified - length bytes
		// starting at wireData, no bytes dropped or substituted.
		if (wireLength <= 0) return;

		RecordBuilder rec;
		constexpr unsigned char msgType = 0;

		rec.put(msgType);
		rec.put(data.messageSequence);
		rec.put(wireLength);
		rec.put(wireData, static_cast<std::size_t>(wireLength));

		EmitRecord(data, rec.bytes());

		++data.messageSequence;
	}

	void ServerDemo::OnTransmit(Game::client_s* cl, const unsigned char* wireData, const int wireLength)
	{
		if (!cl || !wireData || wireLength <= 0) return;

		// Gate exactly the way iw6-mod's SV_Netchan_Transmit hook does: only clients actually
		// receiving meaningful traffic (loading in, or fully active), never bots/test clients.
		if (cl->bIsTestClient) return;
		if (cl->header.state != Game::CS_CLIENTLOADING && cl->header.state != Game::CS_ACTIVE) return;

		const auto clientNum = static_cast<int>(cl - Game::svs_clients);
		if (!ValidClientNum(clientNum)) return;

		// A map change closes and clears EVERY client's buffer/file together, before touching
		// the one this call is for.
		if (CheckMapRotation())
		{
			ResetAll(true);
		}

		auto& data = Clients[clientNum];

		// Reconnect into the same slot: a different lastConnectTime than what we last saw means
		// this is a genuinely new connection wearing the old slot number, not a continuation.
		if (data.lastConnectTime && *data.lastConnectTime != cl->lastConnectTime)
		{
			ResetClient(clientNum, false);
		}
		data.lastConnectTime = cl->lastConnectTime;

		EnsureBufferHeader(clientNum, cl);

		if (!data.firstServerTime)
		{
			data.firstServerTime = static_cast<int>(*Game::svs_time);
		}
		data.curServerTime = static_cast<int>(*Game::svs_time);

		const auto clientLoading = (cl->header.state == Game::CS_CLIENTLOADING);

		// The player's own archived viewpoint is only meaningful once they are actually
		// active in the world - matches the loading-state skip iw6-mod uses for the same
		// reason (there is no predicted playerState worth recording while still loading in).
		if (!clientLoading)
		{
			AppendArchiveRecord(data, cl);
		}
		AppendNetworkRecord(data, wireData, wireLength);

		if (!data.fileActive && SVDemoAutoRecord.get<bool>() && !cl->bIsTestClient)
		{
			StartRecording(clientNum, true);
		}
	}

	bool ServerDemo::SV_Netchan_Transmit_Stub(Game::client_s* client, const void* data, const int length)
	{
		// Capture FIRST, call through SECOND. This used to be the other way around on the
		// theory that "the real send should never be affected by our capture" - true, but it
		// doesn't require reading AFTER the real call: `data` is a reused per-client scratch
		// buffer (confirmed via Ghidra - SV_SendMessageToClient builds each message into the
		// same static buffer every call), and reading it only after SV_Netchan_Transmit has
		// already run risked reading whatever that call left behind rather than the message it
		// was actually handed. OnTransmit only reads and copies - it never touches `data` or
		// `client`, so capturing first changes nothing about the real send either way, and
		// removes any dependency on what SV_Netchan_Transmit does internally with its buffer.
		OnTransmit(client, static_cast<const unsigned char*>(data), length);

		return Utils::Hook::Call<bool(Game::client_s*, const void*, int)>(SV_NETCHAN_TRANSMIT)(client, data, length);
	}

	void ServerDemo::StartRecording(const int clientNum, const bool automatic)
	{
		if (!ValidClientNum(clientNum)) return;

		auto& data = Clients[clientNum];
		if (data.fileActive)
		{
			if (!automatic)
			{
				Logger::Print("[ServerDemo] Client {} is already being recorded\n", clientNum);
			}
			return;
		}

		auto* cl = &Game::svs_clients[clientNum];
		if (cl->header.state < Game::CS_CONNECTED)
		{
			if (!automatic)
			{
				Logger::Print("[ServerDemo] Client {} is not connected\n", clientNum);
			}
			return;
		}

		// Never record bots/test clients.
		if (cl->bIsTestClient)
		{
			if (!automatic)
			{
				Logger::Print("[ServerDemo] Client {} is a bot, not recording\n", clientNum);
			}
			return;
		}

		if (!data.bufferActive)
		{
			// sv_demoAutoRecord (or a manual serverrecord) was enabled/invoked before this
			// client had sent a single message - nothing has been buffered yet. This can only
			// happen in the narrow window between a slot being reused and the first transmit
			// to it; OnTransmit will call back into StartRecording itself as soon as the
			// buffer exists.
			Logger::Print("[ServerDemo] Client {} cannot be recorded yet - no buffered data\n", clientNum);
			return;
		}

		if (automatic)
		{
			CleanupOldAutoDemos();
		}

		auto label = SanitizeName(cl->name);
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
			return;
		}

		// Seed the file with everything already buffered for this client - which, thanks to
		// buffer-everything, is the real, complete, unbroken record of every message sent to
		// them since they connected (or since the last map change/reconnect). Recording that
		// "starts after a couple of minutes" is not starting from a stale cached frame; it is
		// dumping two real minutes of real traffic, headers included, with the delta chain
		// intact the whole way through. No forced keyframe, no cached gamestate to go stale.
		if (!data.buffer.empty())
		{
			Game::FS_WriteToDemo(data.buffer.data(), static_cast<int>(data.buffer.size()), handle);
		}

		data.demoFile = handle;
		data.fileBaseName = baseName;
		data.fileActive = true;
		data.autoRecorded = automatic;
		data.recordStartMs = Game::Sys_Milliseconds();
		std::time(&data.recordStartTimeStamp);

		Logger::Print("[ServerDemo] Started recording client {} ({}) -> {} ({} bytes seeded from buffer)\n",
			clientNum, cl->name, path, data.buffer.size());
	}

	void ServerDemo::StopRecording(const int clientNum)
	{
		if (!ValidClientNum(clientNum)) return;

		auto& data = Clients[clientNum];
		if (!data.fileActive) return;

		WriteSidecar(data, data.fileBaseName);
		Game::FS_FCloseFile(data.demoFile);

		Logger::Print("[ServerDemo] Stopped recording client {}, wrote {}{}.dm_13\n",
			clientNum, DEMO_DIR, data.fileBaseName);

		// Deliberately does NOT reset the buffer/times/counters - only the file. A later
		// serverrecord for the same client, same connection, resumes seeded with everything
		// that happened while not recording, rather than starting from a blank buffer.
		data.demoFile = 0;
		data.fileActive = false;
		data.fileBaseName.clear();
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

	void ServerDemo::StopAll()
	{
		for (auto i = 0; i < static_cast<int>(Game::MAX_CLIENTS); ++i)
		{
			StopRecording(i);
		}
	}

	void ServerDemo::OnClientDisconnected(const int clientNum)
	{
		// Full reset, not just StopRecording: a disconnect ends this connection's buffer
		// history too, so a later reconnect (which OnTransmit would also catch via
		// lastConnectTime) starts clean rather than potentially resuming stale data.
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

		Events::OnClientDisconnect(OnClientDisconnected);

		// The ONE hook: the real SV_Netchan_Transmit, confirmed via Ghidra decompilation as
		// char FUN_0047CB60(client_s*, const void* data, int length), cdecl, with exactly one
		// caller in the whole binary (SV_SendMessageToClient, 0x48FE90) at this call site.
		// Every message to every client - loading or active, gamestate or snapshot or reliable
		// command - passes through here exactly once, already fully compressed. Nothing else
		// in this codebase hooks this address.
		Utils::Hook(SV_NETCHAN_TRANSMIT_CALL_SITE, SV_Netchan_Transmit_Stub, HOOK_CALL).install()->quick();

		Scheduler::OnGameShutdown([]
		{
			ResetAll(false);
		});

		Command::Add("serverrecord", ServerRecordCommand);
		Command::Add("serverstoprecord", ServerStopRecordCommand);
	}
}
