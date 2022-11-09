#include <STDInclude.hpp>
#include "GSC/Script.hpp"

#include <mongoose.h>

namespace Components
{
	static mg_mgr Mgr;

	Download::ClientDownload Download::CLDownload;

	std::thread Download::ServerThread;
	bool Download::Terminate;
	bool Download::ServerRunning;

#pragma region Client

	void Download::InitiateMapDownload(const std::string& map, bool needPassword)
	{
		Download::InitiateClientDownload(map, needPassword, true);
	}

	void Download::InitiateClientDownload(const std::string& mod, bool needPassword, bool map)
	{
		if (Download::CLDownload.running) return;

		Scheduler::Once([]
		{
			Dvar::Var("ui_dl_timeLeft").set(Utils::String::FormatTimeSpan(0));
			Dvar::Var("ui_dl_progress").set("(0/0) %");
			Dvar::Var("ui_dl_transRate").set("0.0 MB/s");
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

			Download::CLDownload.hashedPassword = Utils::String::DumpHex(Utils::Cryptography::SHA256::Compute(password), "");
		}

		Download::CLDownload.running = true;
		Download::CLDownload.isMap = map;
		Download::CLDownload.mod = mod;
		Download::CLDownload.terminateThread = false;
		Download::CLDownload.totalBytes = 0;
		Download::CLDownload.lastTimeStamp = 0;
		Download::CLDownload.downBytes = 0;
		Download::CLDownload.timeStampBytes = 0;
		Download::CLDownload.isPrivate = needPassword;
		Download::CLDownload.target = Party::Target();
		Download::CLDownload.thread = std::thread(Download::ModDownloader, &Download::CLDownload);
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
			Logger::PrintError(Game::CON_CHANNEL_ERROR, "Json Parse Error: {}\n", ex.what());
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

				Download::ClientDownload::File fileEntry;
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
				Logger::PrintError(Game::CON_CHANNEL_ERROR, "Json Error: {}\n", ex.what());
				return false;
			}
		}

		return true;
	}

	bool Download::DownloadFile(ClientDownload* download, unsigned int index)
	{
		if (!download || download->files.size() <= index) return false;

		auto file = download->files[index];

		std::string path = download->mod + "/" + file.name;
		if (download->isMap)
		{
			path = "usermaps/" + path;
		}

		if (Utils::IO::FileExists(path))
		{
			std::string data = Utils::IO::ReadFile(path);

			if (data.size() == file.size && Utils::String::DumpHex(Utils::Cryptography::SHA256::Compute(data), "") == file.hash)
			{
				download->totalBytes += file.size;
				return true;
			}
		}

		auto host = "http://" + download->target.getString();
		auto fastHost = Dvar::Var("sv_wwwBaseUrl").get<std::string>();
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
		if (Dvar::Var("sv_wwwDownload").get<bool>())
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

		Download::FileDownload fDownload;
		fDownload.file = file;
		fDownload.index = index;
		fDownload.download = download;
		fDownload.downloading = true;
		fDownload.receivedBytes = 0;

		Utils::String::Replace(url, " ", "%20");

		download->valid = true;

		fDownload.downloading = true;

		Utils::WebIO webIO;
		webIO.setProgressCallback([&fDownload, &webIO](size_t bytes, size_t)
		{
			if(!fDownload.downloading || fDownload.download->terminateThread)
			{
				webIO.cancelDownload();
				return;
			}

			Download::DownloadProgress(&fDownload, bytes - fDownload.receivedBytes);
		});

		bool result = false;
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
		if (!download) download = &Download::CLDownload;

		std::string host = "http://" + download->target.getString();

		std::string listUrl = host + (download->isMap ? "/map" : "/list") + (download->isPrivate ? ("?password=" + download->hashedPassword) : "");

		std::string list = Utils::WebIO("IW4x", listUrl).setTimeout(5000)->get();
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

		if (!Download::ParseModList(download, list))
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

		for (unsigned int i = 0; i < download->files.size(); ++i)
		{
			if (download->terminateThread) return;

			if (!Download::DownloadFile(download, i))
			{
				if (download->terminateThread) return;

				mod = Utils::String::VA("Failed to download file: %s!", download->files[i].name.data());
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
				const_cast<Game::dvar_t*>(*Game::fs_gameDirVar)->modified = true;

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

#pragma endregion

#pragma region Server
	
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
				Dvar::Var("ui_dl_progress").set(Utils::String::VA("(%d/%d) %d%%", dlIndex, dlSize, dlProgress));
			}, Scheduler::Pipeline::CLIENT);
		}

		int delta = Game::Sys_Milliseconds() - fDownload->download->lastTimeStamp;
		if (delta > 300)
		{
			bool doFormat = fDownload->download->lastTimeStamp != 0;
			fDownload->download->lastTimeStamp = Game::Sys_Milliseconds();

			auto dataLeft = fDownload->download->totalBytes - fDownload->download->downBytes;

			int timeLeft = 0;
			if (fDownload->download->timeStampBytes)
			{
				double timeLeftD = ((1.0 * dataLeft) / fDownload->download->timeStampBytes) * delta;
				timeLeft = static_cast<int>(timeLeftD);
			}

			if (doFormat)
			{
				static size_t dlTsBytes;
				static int dlDelta, dlTimeLeft;
				dlTimeLeft = timeLeft;
				dlDelta = delta;
				dlTsBytes = fDownload->download->timeStampBytes;

				Scheduler::Once([]
				{
					Dvar::Var("ui_dl_timeLeft").set(Utils::String::FormatTimeSpan(dlTimeLeft));
					Dvar::Var("ui_dl_transRate").set(Utils::String::FormatBandwidth(dlTsBytes, dlDelta));
				}, Scheduler::Pipeline::MAIN);
			}

			fDownload->download->timeStampBytes = 0;
		}
	}

	static std::string InfoHandler()
	{
		const auto status = ServerInfo::GetInfo();
		const auto host = ServerInfo::GetHostInfo();

		std::unordered_map<std::string, nlohmann::json> info;
		info["status"] = status.to_json();
		info["host"] = host.to_json();

		std::vector<nlohmann::json> players;

		// Build player list
		for (auto i = 0; i < Game::MAX_CLIENTS; ++i)
		{
			std::unordered_map<std::string, nlohmann::json> playerInfo;
			playerInfo["score"] = 0;
			playerInfo["ping"] = 0;
			playerInfo["name"] = "";

			if ((*Game::com_sv_running)->current.enabled)
			{
				if (Game::svs_clients[i].header.state < Game::CS_CONNECTED) continue;

				playerInfo["score"] = Game::SV_GameClientNum_Score(i);
				playerInfo["ping"] = Game::svs_clients[i].ping;
				playerInfo["name"] = Game::svs_clients[i].name;
			}
			else
			{
				// Score and ping are irrelevant
				const auto* name = Game::PartyHost_GetMemberName(Game::g_lobbyData, i);
				if (!name || *name == '\0') continue;

				playerInfo["name"] = name;
			}

			players.emplace_back(playerInfo);
		}

		info["players"] = players;
		return {nlohmann::json(info).dump()};
	}

	static std::string ListHandler()
	{
		static nlohmann::json jsonList;
		static auto handled = false;

		const std::string fs_gameDirVar = (*Game::fs_gameDirVar)->current.string;

		if (!fs_gameDirVar.empty() && !handled)
		{
			handled = true;

			std::vector<nlohmann::json> fileList;

			const auto path = Dvar::Var("fs_basepath").get<std::string>() + "\\" + fs_gameDirVar;
			auto list = FileSystem::GetSysFileList(path, "iwd", false);
			list.emplace_back("mod.ff");

			for (const auto& file : list)
			{
				std::string filename = path + "\\" + file;
				if (file.find("_svr_") == std::string::npos)
				{
					std::unordered_map<std::string, nlohmann::json> jsonFileList;
					std::string fileBuffer = Utils::IO::ReadFile(filename);
					if (fileBuffer.empty())
					{
						continue;
					}

					jsonFileList["name"] = file;
					jsonFileList["size"] = fileBuffer.size();
					jsonFileList["hash"] = Utils::Cryptography::SHA256::Compute(fileBuffer, true);

					fileList.emplace_back(jsonFileList);
				}
			}

			jsonList = fileList;
		}

		return {jsonList.dump()};
	}

	static std::string MapHandler()
	{
		static std::string mapNamePre;
		static nlohmann::json jsonList;

		const auto mapName = (Party::IsInUserMapLobby() ? Dvar::Var("ui_mapname").get<std::string>() : Maps::GetUserMap()->getName());
		if (!Maps::GetUserMap()->isValid() && !Party::IsInUserMapLobby())
		{
			mapNamePre.clear();
			jsonList = {};
		}
		else if (!mapName.empty() && mapName != mapNamePre)
		{
			std::vector<nlohmann::json> fileList;

			mapNamePre = mapName;

			const auto path = Dvar::Var("fs_basepath").get<std::string>() + "\\usermaps\\" + mapName;

			for (auto i = 0; i < ARRAYSIZE(Maps::UserMapFiles); ++i)
			{
				const auto filename = path + "\\" + mapName + Maps::UserMapFiles[i];
				
				std::map<std::string, nlohmann::json> file;
				std::string fileBuffer = Utils::IO::ReadFile(filename);
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

		return {jsonList.dump()};
	}

	static void FileHandler(mg_connection* c, const mg_http_message* hm)
	{
		std::string url(hm->uri.ptr, hm->uri.len);

		Utils::String::Replace(url, "\\", "/");

		// Strip /file
		url = url.substr(6);
		Utils::String::Replace(url, "%20", " ");

		auto isMap = false;
		if (url.starts_with("map/"))
		{
			isMap = true;
			url = url.substr(4);

			auto mapName = (Party::IsInUserMapLobby() ? Dvar::Var("ui_mapname").get<std::string>() : Maps::GetUserMap()->getName());
			auto isValidFile = false;
			for (auto i = 0; i < ARRAYSIZE(Maps::UserMapFiles); ++i)
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

		std::string file;
		const auto fsGame = Dvar::Var("fs_game").get<std::string>();
		const auto path = Dvar::Var("fs_basepath").get<std::string>() + "\\" + (isMap ? "" : fsGame + "\\") + url;

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

	static void HTMLHandler(mg_connection* c, mg_http_message* hm)
	{
		auto url = "html" + std::string(hm->uri.ptr, hm->uri.len);
		FileSystem::File file;

		if (url.ends_with("/"))
		{
			url.append("index.html");
			file = FileSystem::File(url);
		}
		else
		{
			file = FileSystem::File(url);
			if (!file.exists())
			{
				url.append("/index.html");
				file = FileSystem::File(url);
			}
		}

		const auto mimeType = Utils::GetMimeType(url);

		if (file.exists())
		{
			mg_printf(c, "%s", "HTTP/1.1 200 OK\r\n");
			mg_printf(c, "Content-Type: %s\r\n", mimeType.data());
			mg_printf(c, "Content-Length: %d\r\n", static_cast<int>(file.getBuffer().size()));
			mg_printf(c, "%s", "Connection: close\r\n");
			mg_printf(c, "%s", "\r\n");
			mg_send(c, file.getBuffer().data(), file.getBuffer().size());
		}
		else
		{
			mg_http_reply(c, 404, "Content-Type: text/html\r\n", "404 - Not Found");
		}
	}

	static void EventHandler(mg_connection* c, int ev, void* ev_data, [[maybe_unused]] void* fn_data)
	{
		if (ev != MG_EV_HTTP_MSG)
		{
			return;
		}

		auto* hm = static_cast<mg_http_message*>(ev_data);
		std::string url(hm->uri.ptr, hm->uri.len);

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
			HTMLHandler(c, hm);
		}
	}

#pragma endregion

	Download::Download()
	{
		if (Dedicated::IsEnabled())
		{
			mg_mgr_init(&Mgr);

			Network::OnStart([]
			{
				const auto* nc = mg_http_listen(&Mgr, Utils::String::VA(":%hu", Network::GetPort()), &EventHandler, &Mgr);
				if (!nc)
				{
					Logger::PrintError(Game::CON_CHANNEL_ERROR, "Failed to bind TCP socket, mod download won't work!\n");
				}
			});

			Download::ServerRunning = true;
			Download::Terminate = false;
			Download::ServerThread = std::thread([]
			{
				while (!Download::Terminate)
				{
					mg_mgr_poll(&Mgr, 100);
				}
			});
		}
		else
		{
			Scheduler::Once([]
			{
				Dvar::Register<const char*>("ui_dl_timeLeft", "", Game::DVAR_NONE, "");
				Dvar::Register<const char*>("ui_dl_progress", "", Game::DVAR_NONE, "");
				Dvar::Register<const char*>("ui_dl_transRate", "", Game::DVAR_NONE, "");
			}, Scheduler::Pipeline::MAIN);

			UIScript::Add("mod_download_cancel", []([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info)
			{
				Download::CLDownload.clear();
			});
		}

		Scheduler::Once([]
		{
			Dvar::Register<bool>("sv_wwwDownload", false, Game::DVAR_NONE, "Set to true to enable downloading maps/mods from an external server.");
			Dvar::Register<const char*>("sv_wwwBaseUrl", "", Game::DVAR_NONE, "Set to the base url for the external map download.");
		}, Scheduler::Pipeline::MAIN);

		Script::AddFunction("HttpGet", Script::ShowDeprecationWarning);
		Script::AddFunction("HttpCancel", Script::ShowDeprecationWarning);
	}

	Download::~Download()
	{
		if (Download::ServerRunning)
		{
			mg_mgr_free(&Mgr);
		}
	}

	void Download::preDestroy()
	{
		Download::Terminate = true;
		if (Download::ServerThread.joinable())
		{
			Download::ServerThread.join();
		}

		if (!Dedicated::IsEnabled())
		{
			Download::CLDownload.clear();
		}
	}
}
