#include "STDInclude.hpp"

namespace Components
{
	mg_mgr Download::Mgr;
	Download::ClientDownload Download::CLDownload;
	std::vector<std::shared_ptr<Download::ScriptDownload>> Download::ScriptDownloads;

#pragma region Client

	void Download::InitiateMapDownload(std::string map)
	{
		Download::InitiateClientDownload(map, true);
	}

	void Download::InitiateClientDownload(std::string mod, bool map)
	{
		if (Download::CLDownload.running) return;

		QuickPatch::Once([]()
		{
			Dvar::Var("ui_dl_timeLeft").set(Utils::String::FormatTimeSpan(0));
			Dvar::Var("ui_dl_progress").set("(0/0) %");
			Dvar::Var("ui_dl_transRate").set("0.0 MB/s");
		});

		Command::Execute("openmenu mod_download_popmenu", false);

		Download::CLDownload.running = true;
		Download::CLDownload.isMap = map;
		Download::CLDownload.mod = mod;
		Download::CLDownload.terminateThread = false;
		Download::CLDownload.totalBytes = 0;
		Download::CLDownload.lastTimeStamp = 0;
		Download::CLDownload.downBytes = 0;
		Download::CLDownload.timeStampBytes = 0;
		Download::CLDownload.target = Party::Target();
		Download::CLDownload.thread = std::thread(Download::ModDownloader, &Download::CLDownload);
	}

	bool Download::ParseModList(ClientDownload* download, std::string list)
	{
		if (!download) return false;
		download->files.clear();

		std::string error;
		json11::Json listData = json11::Json::parse(list, error);

		if (!error.empty() || !listData.is_array())
		{
			Logger::Print("Error: %s\n", error.data());
			return false;
		}

		download->totalBytes = 0;

		for (auto& file : listData.array_items())
		{
			if (!file.is_object()) return false;

			auto hash = file["hash"];
			auto name = file["name"];
			auto size = file["size"];

			if (!hash.is_string() || !name.is_string() || !size.is_number()) return false;

			Download::ClientDownload::File fileEntry;
			fileEntry.name = name.string_value();
			fileEntry.hash = hash.string_value();
			fileEntry.size = static_cast<size_t>(size.number_value());

			if (!fileEntry.name.empty())
			{
				download->files.push_back(fileEntry);
				download->totalBytes += fileEntry.size;
			}
		}

		return true;
	}

	void Download::DownloadHandler(mg_connection *nc, int ev, void* ev_data)
	{
		http_message* hm = reinterpret_cast<http_message*>(ev_data);
		Download::FileDownload* fDownload = reinterpret_cast<Download::FileDownload*>(nc->mgr->user_data);

		if (ev == MG_EV_CONNECT)
		{
			if (hm->message.p)
			{
				fDownload->downloading = true;
				return;
			}
		}

		if (ev == MG_EV_RECV)
		{
			size_t bytes = static_cast<size_t>(*reinterpret_cast<int*>(ev_data));
			fDownload->receivedBytes += bytes;
			fDownload->download->downBytes += bytes;
			fDownload->download->timeStampBytes += bytes;

			double progress = 0;
			if (fDownload->download->totalBytes)
			{
				progress = (100.0 / fDownload->download->totalBytes) * fDownload->download->downBytes;
			}

			static unsigned int dlIndex, dlSize, dlProgress;
			dlIndex = fDownload->index + 1;
			dlSize = fDownload->download->files.size();
			dlProgress = static_cast<unsigned int>(progress);

			static bool framePushed = false;

			if (!framePushed)
			{
				framePushed = true;
				QuickPatch::Once([]()
				{
					framePushed = false;
					Dvar::Var("ui_dl_progress").set(Utils::String::VA("(%d/%d) %d%%", dlIndex, dlSize, dlProgress));
				});
			}

			int delta = Game::Sys_Milliseconds() - fDownload->download->lastTimeStamp;
			if (delta > 300)
			{
				bool doFormat = fDownload->download->lastTimeStamp != 0;
				fDownload->download->lastTimeStamp = Game::Sys_Milliseconds();

				size_t dataLeft = fDownload->download->totalBytes - fDownload->download->downBytes;

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

					QuickPatch::Once([]()
					{
						Dvar::Var("ui_dl_timeLeft").set(Utils::String::FormatTimeSpan(dlTimeLeft));
						Dvar::Var("ui_dl_transRate").set(Utils::String::FormatBandwidth(dlTsBytes, dlDelta));
					});
				}

				fDownload->download->timeStampBytes = 0;
			}
		}

		if (ev == MG_EV_HTTP_REPLY)
		{
			nc->flags |= MG_F_CLOSE_IMMEDIATELY;
			fDownload->buffer = std::string(hm->body.p, hm->body.len);
			fDownload->downloading = false;
			return;
		}
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

		std::string url = "http://" + download->target.getString() + "/file/" + (download->isMap ? "map/" : "") + file.name;

		Download::FileDownload fDownload;
		fDownload.file = file;
		fDownload.index = index;
		fDownload.download = download;
		fDownload.downloading = true;
		fDownload.receivedBytes = 0;

		Utils::String::Replace(url, " ", "%20");

		download->valid = true;
		ZeroMemory(&download->mgr, sizeof download->mgr);
		mg_mgr_init(&download->mgr, &fDownload);
		mg_connect_http(&download->mgr, Download::DownloadHandler, url.data(), nullptr, nullptr);

		while (fDownload.downloading && !fDownload.download->terminateThread)
		{
			mg_mgr_poll(&download->mgr, 0);
		}

		mg_mgr_free(&download->mgr);
		download->valid = false;

		if (fDownload.buffer.size() != file.size || Utils::String::DumpHex(Utils::Cryptography::SHA256::Compute(fDownload.buffer), "") != file.hash)
		{
			return false;
		}

		Utils::IO::CreateDir("usermaps/" + download->mod);
		Utils::IO::WriteFile(path, fDownload.buffer);

		return true;
	}

	void Download::ModDownloader(ClientDownload* download)
	{
		if (!download) download = &Download::CLDownload;

		std::string host = "http://" + download->target.getString();

		std::string list = Utils::WebIO("IW4x", host + (download->isMap ? "/map" : "/list")).setTimeout(5000)->get();
		if (list.empty())
		{
			if (download->terminateThread) return;

			download->thread.detach();
			download->clear();

			QuickPatch::Once([]()
			{
				Command::Execute("closemenu mod_download_popmenu");
				Party::ConnectError("Failed to download the modlist!");
			});

			return;
		}

		if (download->terminateThread) return;

		if (!Download::ParseModList(download, list))
		{
			if (download->terminateThread) return;

			download->thread.detach();
			download->clear();

			QuickPatch::Once([]()
			{
				Command::Execute("closemenu mod_download_popmenu");
				Party::ConnectError("Failed to parse the modlist!");
			});

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

				QuickPatch::Once([]()
				{
					Dvar::Var("partyend_reason").set(mod);
					mod.clear();

					Command::Execute("closemenu mod_download_popmenu");
					Command::Execute("openmenu menu_xboxlive_partyended");
				});

				return;
			}
		}

		if (download->terminateThread) return;

		download->thread.detach();
		download->clear();

		if(download->isMap)
		{
			QuickPatch::Once([]()
			{
				Command::Execute("reconnect", false);
			});
		}
		else
		{
			// Run this on the main thread
			QuickPatch::Once([]()
			{
				auto fsGame = Dvar::Var("fs_game");
				fsGame.set(mod);
				fsGame.get<Game::dvar_t*>()->modified = true;
				mod.clear();

				Command::Execute("closemenu mod_download_popmenu", false);

				if (Dvar::Var("cl_modVidRestart").get<bool>())
				{
					Command::Execute("vid_restart", false);
				}

				Command::Execute("reconnect", false);
			});
		}
	}

#pragma endregion

#pragma region Server

	bool Download::IsClient(mg_connection *nc)
	{
		return (Download::GetClient(nc) != nullptr);
	}

	Game::client_t* Download::GetClient(mg_connection *nc)
	{
		Network::Address address(nc->sa.sa);

		for (int i = 0; i < *Game::svs_numclients; ++i)
		{
			Game::client_t* client = &Game::svs_clients[i];

			if (client->state >= 3)
			{
				if (address.getIP().full == Network::Address(client->addr).getIP().full)
				{
					return client;
				}
			}
		}

		return nullptr;
	}

	void Download::Forbid(mg_connection *nc)
	{
		mg_printf(nc, "HTTP/1.1 403 Forbidden\r\n"
			"Content-Type: text/html\r\n"
			"Connection: close\r\n"
			"\r\n"
			"403 - Forbidden");

		nc->flags |= MG_F_SEND_AND_CLOSE;
	}

	void Download::MapHandler(mg_connection *nc, int ev, void* /*ev_data*/)
	{
		// Only handle http requests
		if (ev != MG_EV_HTTP_REQUEST) return;

		static std::string mapnamePre;
		static json11::Json jsonList;

		std::string mapname = Maps::GetUserMap()->getName();
		if(!Maps::GetUserMap()->isValid())
		{
			mapnamePre.clear();
			jsonList = std::vector<json11::Json>();
		}
		else if (!mapname.empty() && mapname != mapnamePre)
		{
			std::vector<json11::Json> fileList;

			mapnamePre = mapname;

			std::string path = Dvar::Var("fs_basepath").get<std::string>() + "\\usermaps\\" + mapname;

			for (int i = 0; i < ARRAYSIZE(Maps::UserMapFiles); ++i)
			{
				std::string filename = path + "\\" + mapname + Maps::UserMapFiles[i];
				if (Utils::IO::FileExists(filename))
				{
					std::map<std::string, json11::Json> file;
					std::string fileBuffer = Utils::IO::ReadFile(filename);

					file["name"] = mapname + Maps::UserMapFiles[i];
					file["size"] = static_cast<int>(fileBuffer.size());
					file["hash"] = Utils::Cryptography::SHA256::Compute(fileBuffer, true);

					fileList.push_back(file);
				}
			}

			jsonList = fileList;
		}

		mg_printf(nc,
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: application/json\r\n"
			"Connection: close\r\n"
			"\r\n"
			"%s", jsonList.dump().data());

		nc->flags |= MG_F_SEND_AND_CLOSE;
	}

	void Download::ListHandler(mg_connection* nc, int ev, void* /*ev_data*/)
	{
		// Only handle http requests
		if (ev != MG_EV_HTTP_REQUEST) return;

// 		if (!Download::IsClient(nc))
// 		{
// 			Download::Forbid(nc);
// 		}
// 		else
		{
			static std::string fsGamePre;
			static json11::Json jsonList;

			std::string fsGame = Dvar::Var("fs_game").get<std::string>();

			if (!fsGame.empty() && fsGame != fsGamePre)
			{
				std::vector<json11::Json> fileList;

				fsGamePre = fsGame;

				std::string path = Dvar::Var("fs_basepath").get<std::string>() + "\\" + fsGame;
				auto list = FileSystem::GetSysFileList(path, "iwd", false);

				list.push_back("mod.ff");

				for (auto i = list.begin(); i != list.end(); ++i)
				{
					std::string filename = path + "\\" + *i;
					if (strstr(i->data(), "_svr_") == nullptr && Utils::IO::FileExists(filename))
					{
						std::map<std::string, json11::Json> file;
						std::string fileBuffer = Utils::IO::ReadFile(filename);

						file["name"] = *i;
						file["size"] = static_cast<int>(fileBuffer.size());
						file["hash"] = Utils::Cryptography::SHA256::Compute(fileBuffer, true);

						fileList.push_back(file);
					}
				}

				jsonList = fileList;
			}

			mg_printf(nc,
				"HTTP/1.1 200 OK\r\n"
				"Content-Type: application/json\r\n"
				"Connection: close\r\n"
				"\r\n"
				"%s", jsonList.dump().data());

			nc->flags |= MG_F_SEND_AND_CLOSE;
		}
	}

	void Download::FileHandler(mg_connection *nc, int ev, void *ev_data)
	{
		// Only handle http requests
		if (ev != MG_EV_HTTP_REQUEST) return;

		http_message* message = reinterpret_cast<http_message*>(ev_data);

// 		if (!Download::IsClient(nc))
// 		{
// 			Download::Forbid(nc);
// 		}
// 		else
		{
			std::string url(message->uri.p, message->uri.len);
			Utils::String::Replace(url, "\\", "/");

			if (url.size() >= 6)
			{
				url = url.substr(6);
			}

			Utils::String::Replace(url, "%20", " ");

			bool isMap = false;
			if (Utils::String::StartsWith(url, "map/"))
			{
				isMap = true;
				url = url.substr(4);

				std::string mapname = Maps::GetUserMap()->getName();

				bool isValidFile = false;
				for (int i = 0; i < ARRAYSIZE(Maps::UserMapFiles); ++i)
				{
					if(url == (mapname + Maps::UserMapFiles[i]))
					{
						isValidFile = true;
						break;
					}
				}

				if(!Maps::GetUserMap()->isValid() || !isValidFile)
				{
					Download::Forbid(nc);
					return;
				}

				url = Utils::String::VA("usermaps\\%s\\%s", mapname.data(), url.data());
			}
			else
			{
				if (url.find_first_of("/") != std::string::npos || (!Utils::String::EndsWith(url, ".iwd") && url != "mod.ff") || strstr(url.data(), "_svr_") != nullptr)
				{
					Download::Forbid(nc);
					return;
				}
			}

			std::string file;
			std::string fsGame = Dvar::Var("fs_game").get<std::string>();
			std::string path = Dvar::Var("fs_basepath").get<std::string>() + "\\" + (isMap ? "" : fsGame + "\\") + url;

			if ((!isMap && fsGame.empty()) || !Utils::IO::ReadFile(path, &file))
			{
				mg_printf(nc,
					"HTTP/1.1 404 Not Found\r\n"
					"Content-Type: text/html\r\n"
					"Connection: close\r\n"
					"\r\n"
					"404 - Not Found %s", path.data());
			}
			else
			{
				mg_printf(nc,
					"HTTP/1.1 200 OK\r\n"
					"Content-Type: application/octet-stream\r\n"
					"Content-Length: %d\r\n"
					"Connection: close\r\n"
					"\r\n", file.size());

				mg_send(nc, file.data(), static_cast<int>(file.size()));
			}

			nc->flags |= MG_F_SEND_AND_CLOSE;
		}
	}

	void Download::InfoHandler(mg_connection* nc, int ev, void* /*ev_data*/)
	{
		// Only handle http requests
		if (ev != MG_EV_HTTP_REQUEST) return;

		//http_message* message = reinterpret_cast<http_message*>(ev_data);

		Utils::InfoString status = ServerInfo::GetInfo();

		std::map<std::string, json11::Json> info;
		info["status"] = status.to_json();

		std::vector<json11::Json> players;

		// Build player list
		for (int i = 0; i < atoi(status.get("sv_maxclients").data()); ++i) // Maybe choose 18 here?
		{
			std::map<std::string, json11::Json> playerInfo;
			playerInfo["score"] = 0;
			playerInfo["ping"] = 0;
			playerInfo["name"] = "";

			if (Dvar::Var("sv_running").get<bool>())
			{
				if (Game::svs_clients[i].state < 3) continue;

				playerInfo["score"] = Game::SV_GameClientNum_Score(i);
				playerInfo["ping"] = Game::svs_clients[i].ping;
				playerInfo["name"] = Game::svs_clients[i].name;
			}
			else
			{
				// Score and ping are irrelevant
				const char* namePtr = Game::PartyHost_GetMemberName(reinterpret_cast<Game::PartyData_t*>(0x1081C00), i);
				if (!namePtr || !namePtr[0]) continue;

				playerInfo["name"] = namePtr;
			}

			players.push_back(playerInfo);
		}

		info["players"] = players;

		mg_printf(nc,
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: application/json\r\n"
			"Connection: close\r\n"
			"Access-Control-Allow-Origin: *\r\n"
			"\r\n"
			"%s", json11::Json(info).dump().data());

		nc->flags |= MG_F_SEND_AND_CLOSE;
	}

	void Download::EventHandler(mg_connection *nc, int ev, void *ev_data)
	{
		// Only handle http requests
		if (ev != MG_EV_HTTP_REQUEST) return;

		http_message* message = reinterpret_cast<http_message*>(ev_data);

// 		if (message->uri.p, message->uri.len == "/"s)
// 		{
// 			mg_printf(nc,
// 				"HTTP/1.1 200 OK\r\n"
// 				"Content-Type: text/html\r\n"
// 				"Connection: close\r\n"
// 				"\r\n"
// 				"Hi fella!<br>You are%s connected to this server!", (Download::IsClient(nc) ? " " : " not"));
//
//				Game::client_t* client = Download::GetClient(nc);
//
// 			if (client)
// 			{
// 				mg_printf(nc, "<br>Hello %s!", client->name);
// 			}
// 		}
// 		else
		{
			//std::string path = (Dvar::Var("fs_basepath").get<std::string>() + "\\" BASEGAME "\\html");
			//mg_serve_http_opts opts = { 0 };
			//opts.document_root = path.data();
			//mg_serve_http(nc, message, opts);

			FileSystem::File file;
			std::string url = "html" + std::string(message->uri.p, message->uri.len);

			if (Utils::String::EndsWith(url, "/"))
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

			std::string mimeType = Utils::GetMimeType(url);

			if (file.exists())
			{
				std::string buffer = file.getBuffer();

				mg_printf(nc,
					"HTTP/1.1 200 OK\r\n"
					"Content-Type: %s\r\n"
					"Content-Length: %d\r\n"
					"Connection: close\r\n"
					"\r\n", mimeType.data(), buffer.size());

				mg_send(nc, buffer.data(), static_cast<int>(buffer.size()));
			}
			else
			{
				mg_printf(nc,
					"HTTP/1.1 404 Not Found\r\n"
					"Content-Type: text/html\r\n"
					"Connection: close\r\n"
					"\r\n"
					"404 - Not Found");
			}
		}

		nc->flags |= MG_F_SEND_AND_CLOSE;
	}

#pragma endregion

	Download::Download()
	{
		if (Dedicated::IsEnabled())
		{
			ZeroMemory(&Download::Mgr, sizeof Download::Mgr);
			mg_mgr_init(&Download::Mgr, nullptr);

			Network::OnStart([] ()
			{
				mg_connection* nc = mg_bind(&Download::Mgr, Utils::String::VA("%hu", (Dvar::Var("net_port").get<int>() & 0xFFFF)), Download::EventHandler);

				if (nc)
				{
					// Handle special requests
					mg_register_http_endpoint(nc, "/info", Download::InfoHandler);
					mg_register_http_endpoint(nc, "/list", Download::ListHandler);
					mg_register_http_endpoint(nc, "/map",  Download::MapHandler);
					mg_register_http_endpoint(nc, "/file/", Download::FileHandler);

					mg_set_protocol_http_websocket(nc);
				}
				else
				{
					Logger::Print("Failed to bind TCP socket, moddownload won't work!\n");
				}
			});

			QuickPatch::OnFrame([]
			{
				mg_mgr_poll(&Download::Mgr, 0);
			});
		}
		else
		{
			Dvar::OnInit([]()
			{
				Dvar::Register<const char*>("ui_dl_timeLeft", "", Game::dvar_flag::DVAR_FLAG_NONE, "");
				Dvar::Register<const char*>("ui_dl_progress", "", Game::dvar_flag::DVAR_FLAG_NONE, "");
				Dvar::Register<const char*>("ui_dl_transRate", "", Game::dvar_flag::DVAR_FLAG_NONE, "");
			});

			UIScript::Add("mod_download_cancel", [] (UIScript::Token)
			{
				Download::CLDownload.clear();
			});
		}

		QuickPatch::OnFrame([]()
		{
			int workingCount = 0;

			for(auto i = Download::ScriptDownloads.begin(); i != Download::ScriptDownloads.end();)
			{
				auto download = *i;

				if(download->isDone())
				{
					download->notifyDone();
					i = Download::ScriptDownloads.erase(i);
					continue;
				}

				if (download->isWorking())
				{
					download->notifyProgress();
					++workingCount;
				}

				++i;
			}

			for(auto& download : Download::ScriptDownloads)
			{
				if (workingCount > 5) break;
				if(!download->isWorking())
				{
					download->startWorking();
					++workingCount;
				}
			}
		});

		Script::OnVMShutdown([]()
		{
			Download::ScriptDownloads.clear();
		});

		if (Dedicated::IsEnabled() || Flags::HasFlag("scriptablehttp"))
		{
			Script::AddFunction("httpGet", [](Game::scr_entref_t)
			{
				if (Game::Scr_GetNumParam() < 1) return;

				std::string url = Game::Scr_GetString(0);
				unsigned int object = Game::AllocObject();

				Game::Scr_AddObject(object);

				Download::ScriptDownloads.push_back(std::make_shared<ScriptDownload>(url, object));
				Game::RemoveRefToObject(object);
			});

			Script::AddFunction("httpCancel", [](Game::scr_entref_t)
			{
				if (Game::Scr_GetNumParam() < 1) return;

				unsigned int object = Game::Scr_GetObject(0);
				for (auto& download : Download::ScriptDownloads)
				{
					if (object == download->getObject())
					{
						download->cancel();
						break;
					}
				}
			});
		}
	}

	Download::~Download()
	{
		if (Dedicated::IsEnabled())
		{
			mg_mgr_free(&Download::Mgr);
		}
	}

	void Download::preDestroy()
	{
		if (!Dedicated::IsEnabled())
		{
			Download::CLDownload.clear();
		}

		Download::ScriptDownloads.clear();
	}
}
