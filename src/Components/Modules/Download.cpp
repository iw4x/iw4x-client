#include <STDInclude.hpp>
#include <Utils/InfoString.hpp>
#include <Utils/WebIO.hpp>

#include "Download.hpp"
#include "Events.hpp"
#include "MapRotation.hpp"
#include "Party.hpp"
#include "ServerInfo.hpp"

#include <mongoose.h>

#define MG_OVERRIDE_LOG_FN

namespace Components
{
	static mg_mgr Mgr;

	Dvar::Var Download::SV_wwwDownload;
	Dvar::Var Download::SV_wwwBaseUrl;

	Dvar::Var Download::UIDlTimeLeft;
	Dvar::Var Download::UIDlProgress;
	Dvar::Var Download::UIDlTransRate;

	Download::ClientDownload Download::CLDownload;

	std::thread Download::ServerThread;
	volatile bool Download::Terminate;
	bool Download::ServerRunning;

	std::string Download::MongooseLogBuffer;

#pragma region Client

	void Download::InitiateMapDownload(const std::string& map, bool needPassword)
	{
		InitiateClientDownload(map, needPassword, true);
	}

	void Download::InitiateClientDownload(const std::string& mod, bool needPassword, bool map)
	{
		if (CLDownload.running) return;

		Scheduler::Once([]
		{
			UIDlTimeLeft.set(Utils::String::FormatTimeSpan(0));
			UIDlProgress.set("(0/0) %");
			UIDlTransRate.set("0.0 MB/s");
		}, Scheduler::Pipeline::MAIN);

		Command::Execute("openmenu mod_download_popmenu", false);

		if (needPassword)
		{
			const auto password = Dvar::Var("password").get<std::string>();
			if (password.empty())
			{
				// shouldn't ever happen but this is safe
				Party::ConnectError("A password is required to connect to this server!");
				return;
			}

			CLDownload.hashedPassword = Utils::String::DumpHex(Utils::Cryptography::SHA256::Compute(password), "");
		}

		CLDownload.running = true;
		CLDownload.isMap = map;
		CLDownload.mod = mod;
		CLDownload.terminateThread = false;
		CLDownload.totalBytes = 0;
		CLDownload.lastTimeStamp = 0;
		CLDownload.downBytes = 0;
		CLDownload.timeStampBytes = 0;
		CLDownload.isPrivate = needPassword;
		CLDownload.target = Party::Target();
		CLDownload.thread = std::thread(ModDownloader, &CLDownload);
	}

	bool Download::ParseModList(ClientDownload* download, const std::string& list)
	{
		if (!download) return false;
		download->files.clear();

		nlohmann::json listData;
		try
		{
			listData = nlohmann::json::parse(list);
		}
		catch (const nlohmann::json::parse_error& ex)
		{
			Logger::PrintError(Game::CON_CHANNEL_ERROR, "JSON Parse Error: {}\n", ex.what());
			return false;
		}

		if (!listData.is_array())
		{
			return false;
		}

		download->totalBytes = 0;
		const nlohmann::json::array_t listDataArray = listData;

		for (auto& file : listDataArray)
		{
			if (!file.is_object()) return false;

			try
			{
				const auto hash = file.at("hash").get<std::string>();
				const auto name = file.at("name").get<std::string>();
				const auto size = file.at("size").get<std::size_t>();

				ClientDownload::File fileEntry;
				fileEntry.name = name;
				fileEntry.hash = hash;
				fileEntry.size = size;

				if (!fileEntry.name.empty())
				{
					download->files.push_back(fileEntry);
					download->totalBytes += fileEntry.size;
				}
			}
			catch (const nlohmann::json::exception& ex)
			{
				Logger::PrintError(Game::CON_CHANNEL_ERROR, "JSON Error: {}\n", ex.what());
				return false;
			}
		}

		return true;
	}

	bool Download::DownloadFile(ClientDownload* download, unsigned int index)
	{
		if (!download || download->files.size() <= index) return false;

		auto file = download->files[index];

		auto path = download->mod + "/" + file.name;
		if (download->isMap)
		{
			path = "usermaps/" + path;
		}

		if (Utils::IO::FileExists(path))
		{
			auto data = Utils::IO::ReadFile(path);
			if (data.size() == file.size && Utils::String::DumpHex(Utils::Cryptography::SHA256::Compute(data), "") == file.hash)
			{
				download->totalBytes += file.size;
				return true;
			}
		}

		auto host = "http://" + download->target.getString();
		auto fastHost = SV_wwwBaseUrl.get<std::string>();
		if (Utils::String::StartsWith(fastHost, "https://"))
		{
			download->thread.detach();
			download->clear();

			Scheduler::Once([]
			{
				Command::Execute("closemenu mod_download_popmenu");
				Party::ConnectError("HTTPS not supported for downloading!");
			}, Scheduler::Pipeline::CLIENT);

			return false;
		}

		if (!Utils::String::StartsWith(fastHost, "http://"))
		{
			fastHost = "http://" + fastHost;
		}

		std::string url;

		// file directory for fasthost looks like this
		// /-usermaps
		//  /-mp_test
		//    -mp_test.ff
		//    -mp_test.iwd
		//   /-mp_whatever
		//	  /-mp_whatever.ff
		// /-mods
		//  /-mod1
		//	  -mod1.iwd
		//    -mod.ff
		//  /-mod2
		//     ...
		if (SV_wwwDownload.get<bool>())
		{
			if (!Utils::String::EndsWith(fastHost, "/")) fastHost.append("/");
			url = fastHost + path;
		}
		else
		{
			url = host + "/file/" + (download->isMap ? "map/" : "") + file.name
				+ (download->isPrivate ? ("?password=" + download->hashedPassword) : "");
		}

		Logger::Print("Downloading from url {}\n", url);

		FileDownload fDownload;
		fDownload.file = file;
		fDownload.index = index;
		fDownload.download = download;
		fDownload.downloading = true;
		fDownload.receivedBytes = 0;

		Utils::String::Replace(url, " ", "%20");

		download->valid = true;

		fDownload.downloading = true;

		Utils::WebIO webIO;
		webIO.setProgressCallback([&fDownload, &webIO](std::size_t bytes, std::size_t)
		{
			if(!fDownload.downloading || fDownload.download->terminateThread)
			{
				webIO.cancelDownload();
				return;
			}

			DownloadProgress(&fDownload, bytes - fDownload.receivedBytes);
		});

		auto result = false;
		fDownload.buffer = webIO.get(url, &result);
		if (!result) fDownload.buffer.clear();

		fDownload.downloading = false;

		download->valid = false;

		if (fDownload.buffer.size() != file.size || Utils::Cryptography::SHA256::Compute(fDownload.buffer, true) != file.hash)
		{
			return false;
		}

		if (download->isMap) Utils::IO::CreateDir("usermaps/" + download->mod);
		Utils::IO::WriteFile(path, fDownload.buffer);

		return true;
	}

	void Download::ModDownloader(ClientDownload* download)
	{
		if (!download) download = &CLDownload;

		const auto host = "http://" + download->target.getString();

		const auto listUrl = host + (download->isMap ? "/map" : "/list") + (download->isPrivate ? ("?password=" + download->hashedPassword) : "");

		const auto list = Utils::WebIO("IW4x", listUrl).setTimeout(5000)->get();
		if (list.empty())
		{
			if (download->terminateThread) return;

			download->thread.detach();
			download->clear();

			Scheduler::Once([]
			{
				Command::Execute("closemenu mod_download_popmenu");
				Party::ConnectError("Failed to download the modlist!");
			}, Scheduler::Pipeline::CLIENT);

			return;
		}

		if (download->terminateThread) return;

		if (!ParseModList(download, list))
		{
			if (download->terminateThread) return;

			download->thread.detach();
			download->clear();

			Scheduler::Once([]
			{
				Command::Execute("closemenu mod_download_popmenu");
				Party::ConnectError("Failed to parse the modlist!");
			}, Scheduler::Pipeline::CLIENT);

			return;
		}

		if (download->terminateThread) return;

		static std::string mod;
		mod = download->mod;

		for (std::size_t i = 0; i < download->files.size(); ++i)
		{
			if (download->terminateThread) return;

			if (!DownloadFile(download, i))
			{
				if (download->terminateThread) return;

				mod = std::format("Failed to download file: {}!", download->files[i].name);
				download->thread.detach();
				download->clear();

				Scheduler::Once([]
				{
					Dvar::Var("partyend_reason").set(mod);
					mod.clear();

					Command::Execute("closemenu mod_download_popmenu");
					Command::Execute("openmenu menu_xboxlive_partyended");
				}, Scheduler::Pipeline::CLIENT);

				return;
			}
		}

		if (download->terminateThread) return;

		download->thread.detach();
		download->clear();

		if (download->isMap)
		{
			Scheduler::Once([]
			{
				Command::Execute("reconnect", false);
			}, Scheduler::Pipeline::CLIENT);
		}
		else
		{
			// Run this on the main thread
			Scheduler::Once([]
			{
				Game::Dvar_SetString(*Game::fs_gameDirVar, mod.data());
				const_cast<Game::dvar_t*>((*Game::fs_gameDirVar))->modified = true;

				mod.clear();

				Command::Execute("closemenu mod_download_popmenu", false);

				if (Dvar::Var("cl_modVidRestart").get<bool>())
				{
					Command::Execute("vid_restart", false);
				}

				Command::Execute("reconnect", false);
			}, Scheduler::Pipeline::MAIN);
		}
	}

	void Download::DownloadProgress(FileDownload* fDownload, std::size_t bytes)
	{
		fDownload->receivedBytes += bytes;
		fDownload->download->downBytes += bytes;
		fDownload->download->timeStampBytes += bytes;

		static volatile bool framePushed = false;

		if (!framePushed)
		{
			double progress = 0;
			if (fDownload->download->totalBytes)
			{
				progress = (100.0 / fDownload->download->totalBytes) * fDownload->download->downBytes;
			}

			static std::uint32_t dlIndex, dlSize, dlProgress;
			dlIndex = fDownload->index + 1;
			dlSize = fDownload->download->files.size();
			dlProgress = static_cast<std::uint32_t>(progress);

			framePushed = true;
			Scheduler::Once([]
			{
				framePushed = false;
				UIDlProgress.set(std::format("({}/{}) {}%", dlIndex, dlSize, dlProgress));
			}, Scheduler::Pipeline::MAIN);
		}

		auto delta = Game::Sys_Milliseconds() - fDownload->download->lastTimeStamp;
		if (delta > 300)
		{
			const auto doFormat = fDownload->download->lastTimeStamp != 0;
			fDownload->download->lastTimeStamp = Game::Sys_Milliseconds();

			const auto dataLeft = fDownload->download->totalBytes - fDownload->download->downBytes;

			int timeLeft = 0;
			if (fDownload->download->timeStampBytes)
			{
				const double timeLeftD = ((1.0 * dataLeft) / fDownload->download->timeStampBytes) * delta;
				timeLeft = static_cast<int>(timeLeftD);
			}

			if (doFormat)
			{
				static std::size_t dlTsBytes;
				static int dlDelta, dlTimeLeft;
				dlTimeLeft = timeLeft;
				dlDelta = delta;
				dlTsBytes = fDownload->download->timeStampBytes;

				Scheduler::Once([]
				{
					UIDlTimeLeft.set(Utils::String::FormatTimeSpan(dlTimeLeft));
					UIDlTransRate.set(Utils::String::FormatBandwidth(dlTsBytes, dlDelta));
				}, Scheduler::Pipeline::MAIN);
			}

			fDownload->download->timeStampBytes = 0;
		}
	}

#pragma endregion

#pragma region Server

	void Download::LogFn(char c, [[maybe_unused]] void* param)
	{
		// Truncate & print if buffer is 1024 characters in length or otherwise only print when we reached a 'new line'
		if (!std::isprint(static_cast<unsigned char>(c)) || MongooseLogBuffer.size() == 1024)
		{
			Logger::Print(Game::CON_CHANNEL_NETWORK, "{}\n", MongooseLogBuffer);
			MongooseLogBuffer.clear();
			return;
		}

		MongooseLogBuffer.push_back(c);
	}

	static std::string InfoHandler()
	{
		const auto status = ServerInfo::GetInfo();
		const auto host = ServerInfo::GetHostInfo();

		std::unordered_map<std::string, nlohmann::json> info;
		info["status"] = status.to_json();
		info["host"] = host.to_json();
		info["map_rotation"] = MapRotation::to_json();
		info["dedicated"] = Dedicated::com_dedicated->current.integer;

		std::vector<nlohmann::json> players;

		// Build player list
		for (auto i = 0; i < Game::MAX_CLIENTS; ++i)
		{
			std::unordered_map<std::string, nlohmann::json> playerInfo;
			// Insert default values
			playerInfo["score"] = 0;
			playerInfo["ping"] = 0;
			playerInfo["name"] = "Unknown Soldier";
			playerInfo["test_client"] = 0;

			if (Dedicated::IsRunning())
			{
				if (Game::svs_clients[i].header.state < Game::CS_ACTIVE) continue;
				if (!Game::svs_clients[i].gentity || !Game::svs_clients[i].gentity->client) continue;

				playerInfo["score"] = Game::SV_GameClientNum_Score(i);
				playerInfo["ping"] = Game::svs_clients[i].ping;
				playerInfo["name"] = Game::svs_clients[i].name;
				playerInfo["test_client"] = Game::svs_clients[i].bIsTestClient;
			}
			else
			{
				// Score and ping are irrelevant
				const auto* name = Game::PartyHost_GetMemberName(Game::g_lobbyData, i);
				if (!name || !*name) continue;

				playerInfo["name"] = name;
			}

			players.emplace_back(playerInfo);
		}

		info["players"] = players;
		return nlohmann::json(info).dump();
	}

	static std::string ListHandler()
	{
		static nlohmann::json jsonList;
		static std::filesystem::path fsGamePre;

		const std::filesystem::path fsGame = (*Game::fs_gameDirVar)->current.string;

		if (!fsGame.empty() && (fsGamePre != fsGame))
		{
			fsGamePre = fsGame;

			std::vector<nlohmann::json> fileList;

			const auto path = (*Game::fs_basepath)->current.string / fsGame;
			auto list = FileSystem::GetSysFileList(path.generic_string(), "iwd", false);
			list.emplace_back("mod.ff");

			for (const auto& file : list)
			{
				auto filename = path / file;

				if (file.find("_svr_") != std::string::npos) // Files that are 'server only' are skipped
				{
					continue;
				}

				auto fileBuffer = Utils::IO::ReadFile(filename.generic_string());
				if (fileBuffer.empty())
				{
					continue;
				}

				std::unordered_map<std::string, nlohmann::json> jsonFileList;
				jsonFileList["name"] = file;
				jsonFileList["size"] = fileBuffer.size();
				jsonFileList["hash"] = Utils::Cryptography::SHA256::Compute(fileBuffer, true);

				fileList.emplace_back(jsonFileList);
			}

			jsonList = fileList;
		}

		return jsonList.dump();
	}

	static std::string MapHandler()
	{
		static std::string mapNamePre;
		static nlohmann::json jsonList;

		const std::string mapName = Party::IsInUserMapLobby() ? (*Game::ui_mapname)->current.string : Maps::GetUserMap()->getName();
		if (!Maps::GetUserMap()->isValid() && !Party::IsInUserMapLobby())
		{
			mapNamePre.clear();
			jsonList = {};
		}
		else if (!mapName.empty() && mapName != mapNamePre)
		{
			std::vector<nlohmann::json> fileList;

			mapNamePre = mapName;

			const std::filesystem::path basePath = (*Game::fs_basepath)->current.string;
			const auto path = basePath / "usermaps" / mapName;

			for (std::size_t i = 0; i < ARRAYSIZE(Maps::UserMapFiles); ++i)
			{
				const auto filename = std::format("{}\\{}{}", path.generic_string(), mapName, Maps::UserMapFiles[i]);

				std::unordered_map<std::string, nlohmann::json> file;
				auto fileBuffer = Utils::IO::ReadFile(filename);
				if (fileBuffer.empty())
				{
					continue;
				}

				file["name"] = mapName + Maps::UserMapFiles[i];
				file["size"] = fileBuffer.size();
				file["hash"] = Utils::Cryptography::SHA256::Compute(fileBuffer, true);

				fileList.emplace_back(file);
			}

			jsonList = fileList;
		}

		return jsonList.dump();
	}

	static void FileHandler(mg_connection* c, const mg_http_message* hm)
	{
		std::string url(hm->uri.ptr, hm->uri.len);

		Utils::String::Replace(url, "\\", "/");

		url = url.substr(6); // Strip /file
		Utils::String::Replace(url, "%20", " ");

		auto isMap = false;
		if (url.starts_with("map/"))
		{
			isMap = true;
			url = url.substr(4); // Strip map/

			std::string mapName = (Party::IsInUserMapLobby() ? (*Game::ui_mapname)->current.string : Maps::GetUserMap()->getName());
			auto isValidFile = false;
			for (std::size_t i = 0; i < ARRAYSIZE(Maps::UserMapFiles); ++i)
			{
				if (url == (mapName + Maps::UserMapFiles[i]))
				{
					isValidFile = true;
					break;
				}
			}

			if ((!Maps::GetUserMap()->isValid() && !Party::IsInUserMapLobby()) || !isValidFile)
			{
				mg_http_reply(c, 403, "Content-Type: text/html\r\n", "%s", "403 - Forbidden");
				return;
			}

			url = std::format("usermaps\\{}\\{}", mapName, url);
		}
		else
		{
			if ((!url.ends_with(".iwd") && url != "mod.ff") || url.find("_svr_") != std::string::npos)
			{
				mg_http_reply(c, 403, "Content-Type: text/html\r\n", "%s", "403 - Forbidden");
				return;
			}
		}

		const std::string fsGame = (*Game::fs_gameDirVar)->current.string;
		const auto path = std::format("{}\\{}{}", (*Game::fs_basepath)->current.string, isMap ? ""s : (fsGame + "\\"s), url);

		std::string file;
		if ((!isMap && fsGame.empty()) || !Utils::IO::ReadFile(path, &file))
		{
			mg_http_reply(c, 404, "Content-Type: text/html\r\n", "404 - Not Found %s", path.data());
		}
		else
		{
			mg_printf(c, "%s", "HTTP/1.1 200 OK\r\n");
			mg_printf(c, "%s", "Content-Type: application/octet-stream\r\n");
			mg_printf(c, "Content-Length: %d\r\n", static_cast<int>(file.size()));
			mg_printf(c, "%s", "Connection: close\r\n");
			mg_printf(c, "%s", "\r\n");
			mg_send(c, file.data(), file.size());
		}
	}

	static void EventHandler(mg_connection* c, const int ev, void* ev_data, [[maybe_unused]] void* fn_data)
	{
		if (ev != MG_EV_HTTP_MSG)
		{
			return;
		}

		auto* hm = static_cast<mg_http_message*>(ev_data);
		const std::string url(hm->uri.ptr, hm->uri.len);

		if (url.starts_with("/info"))
		{
			const auto reply = InfoHandler();
			mg_http_reply(c, 200, "Content-Type: application/json\r\n", "%s", reply.data());
		}
		else if (url.starts_with("/list"))
		{
			const auto reply = ListHandler();
			mg_http_reply(c, 200, "Content-Type: application/json\r\n", "%s", reply.data());
		}
		else if (url.starts_with("/map"))
		{
			const auto reply = MapHandler();
			mg_http_reply(c, 200, "Content-Type: application/json\r\n", "%s", reply.data());
		}
		else if (url.starts_with("/file"))
		{
			FileHandler(c, hm);
		}
		else
		{
			mg_http_serve_opts opts = { .root_dir = "iw4x/html" }; // Serve local dir
			mg_http_serve_dir(c, hm, &opts);
		}

		c->is_resp = FALSE; // This is important, the lack of this line of code will make the server die (in-game)
		c->is_draining = TRUE;
	}

#pragma endregion

	Download::Download()
	{
		AssertSize(Game::va_info_t, 0x804);
		AssertSize(jmp_buf, 0x40);
		AssertSize(Game::TraceThreadInfo, 0x8);

		if (Dedicated::IsEnabled())
		{
			if (!Flags::HasFlag("disable-mongoose"))
			{
#ifdef _DEBUG
				mg_log_set(MG_LL_INFO);
#else
				mg_log_set(MG_LL_ERROR);
#endif

#ifdef MG_OVERRIDE_LOG_FN
				mg_log_set_fn(LogFn, nullptr);
#endif

				mg_mgr_init(&Mgr);

				Events::OnNetworkInit([]() -> void
				{
					const auto* nc = mg_http_listen(&Mgr, Utils::String::VA(":%hu", Network::GetPort()), &EventHandler, &Mgr);
					if (!nc)
					{
						Logger::PrintError(Game::CON_CHANNEL_ERROR, "Failed to bind TCP socket, mod download won't work!\n");
						Terminate = true;
					}
				});

				ServerRunning = true;
				Terminate = false;
				ServerThread = Utils::Thread::CreateNamedThread("Mongoose", []() -> void
				{
					Com_InitThreadData();

					while (!Terminate)
					{
						mg_mgr_poll(&Mgr, 1000);
					}
				});
			}
		}
		else
		{
			Events::OnDvarInit([]() -> void
			{
				UIDlTimeLeft = Dvar::Register<const char*>("ui_dl_timeLeft", "", Game::DVAR_NONE, "");
				UIDlProgress = Dvar::Register<const char*>("ui_dl_progress", "", Game::DVAR_NONE, "");
				UIDlTransRate = Dvar::Register<const char*>("ui_dl_transRate", "", Game::DVAR_NONE, "");
			});

			UIScript::Add("mod_download_cancel", []([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info)
			{
				CLDownload.clear();
			});
		}

		Events::OnDvarInit([]
		{
			SV_wwwDownload = Dvar::Register<bool>("sv_wwwDownload", false, Game::DVAR_NONE, "Set to true to enable downloading maps/mods from an external server.");
			SV_wwwBaseUrl = Dvar::Register<const char*>("sv_wwwBaseUrl", "", Game::DVAR_NONE, "Set to the base url for the external map download.");
		});
	}

	Download::~Download()
	{
		if (ServerRunning)
		{
			mg_mgr_free(&Mgr);
		}
	}

	void Download::preDestroy()
	{
		Terminate = true;
		if (ServerThread.joinable())
		{
			ServerThread.join();
		}

		if (!Dedicated::IsEnabled())
		{
			CLDownload.clear();
		}
	}
}
