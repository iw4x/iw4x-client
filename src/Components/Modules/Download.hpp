#pragma once

namespace Components
{
	class Download : public Component
	{
	public:
		Download();
		~Download();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* getName() override { return "Download"; };
#endif

		void preDestroy() override;

		static void InitiateClientDownload(std::string mod, bool map = false);
		static void InitiateMapDownload(std::string map);

	private:
		class ClientDownload
		{
		public:
			ClientDownload(bool _isMap = false) : running(false), valid(false), terminateThread(false), isMap(_isMap), totalBytes(0), downBytes(0), lastTimeStamp(0), timeStampBytes(0) {}
			~ClientDownload() { this->clear(); }

			bool running;
			bool valid;
			bool terminateThread;
			bool isMap;
			mg_mgr mgr;
			Network::Address target;
			std::string mod;
			std::thread thread;

			size_t totalBytes;
			size_t downBytes;

			int lastTimeStamp;
			size_t timeStampBytes;

			class File
			{
			public:
				std::string name;
				std::string hash;
				size_t size;
			};

			std::vector<File> files;

			void clear()
			{
				this->terminateThread = true;

				if (this->thread.joinable())
				{
					this->thread.join();
				}

				this->running = false;
				this->mod.clear();
				this->files.clear();

				if (this->valid)
				{
					this->valid = false;
					mg_mgr_free(&(this->mgr));
				}
			}
		};

		class FileDownload
		{
		public:
			ClientDownload* download;
			ClientDownload::File file;

			int timestamp;
			bool downloading;
			unsigned int index;
			std::string buffer;
			size_t receivedBytes;
		};

		class ScriptDownload
		{
		public:
			ScriptDownload(std::string _url, unsigned int _object) : url(_url), object(_object), mgr(new mg_mgr), totalSize(0), receivedSize(0), done(false)
			{
				Game::AddRefToObject(this->getObject());

				ZeroMemory(this->getMgr(), sizeof(*this->getMgr()));
				mg_mgr_init(this->getMgr(), this);
				mg_connect_http(this->getMgr(), ScriptDownload::Handler, this->getUrl().data(), nullptr, nullptr);
			}

			ScriptDownload(ScriptDownload&& other) noexcept
			{
				this->operator=(std::move(other));
			}

			ScriptDownload& operator=(ScriptDownload&& other) noexcept
			{
				this->object = other.object;
				this->done = other.done;
				this->totalSize = other.totalSize;
				this->receivedSize = other.receivedSize;
				this->url = std::move(other.url);
				this->mgr = std::move(other.mgr);

				this->getMgr()->user_data = this;

				other.object = 0;

				return *this;
			}

			~ScriptDownload()
			{
				if (this->getObject())
				{
					Game::RemoveRefToObject(this->getObject());
					this->object = 0;

					mg_mgr_free(this->getMgr());
				}
			}

			void work()
			{
				mg_mgr_poll(this->getMgr(), 0);
			}

			void notifyProgress(size_t bytes)
			{
				this->receivedSize += bytes;

				Game::Scr_AddInt(static_cast<int>(this->totalSize));
				Game::Scr_AddInt(static_cast<int>(this->receivedSize));
				Game::Scr_NotifyId(this->getObject(), Game::SL_GetString("progress", 0), 2);
			}

			void setSize(char* buf, size_t len)
			{
				if(buf && !this->totalSize)
				{
					std::string data(buf, len);
					auto pos = data.find("Content-Length: ");
					if(pos != std::string::npos)
					{
						data = data.substr(pos + 16);

						pos = data.find("\r\n");
						if (pos != std::string::npos)
						{
							data = data.substr(0, pos);

							this->totalSize = size_t(atoll(data.data()));
							return;
						}
					}

					this->totalSize = ~0ul;
				}
			}

			void notifyDone(bool success, std::string result)
			{
				if (this->isDone()) return;

				Game::Scr_AddString(result.data()); // No binary data supported yet
				Game::Scr_AddInt(success);
				Game::Scr_NotifyId(this->getObject(), Game::SL_GetString("done", 0), 2);

				this->done = true;
			}

			bool isDone() { return this->done; };

			std::string getUrl() { return this->url; }
			mg_mgr* getMgr() { return this->mgr.get(); }
			unsigned int getObject() { return this->object; }

		private:
			std::string url;
			unsigned int object;
			std::shared_ptr<mg_mgr> mgr;

			size_t totalSize;
			size_t receivedSize;

			bool done;

			static void Handler(mg_connection *nc, int ev, void* ev_data)
			{
				http_message* hm = reinterpret_cast<http_message*>(ev_data);
				ScriptDownload* object = reinterpret_cast<ScriptDownload*>(nc->mgr->user_data);

				if (ev == MG_EV_RECV)
				{
					object->setSize(nc->recv_mbuf.buf, nc->recv_mbuf.len);

					size_t bytes = static_cast<size_t>(*reinterpret_cast<int*>(ev_data));
					object->notifyProgress(bytes);
				}
				else if (ev == MG_EV_HTTP_REPLY)
				{
					object->notifyDone(true, std::string(hm->body.p, hm->body.len));
				}
				else if(ev == MG_EV_CONNECT)
				{
					if (*static_cast<int*>(ev_data))
					{
						object->notifyDone(false, std::string());
					}
				}
			}
		};

		static mg_mgr Mgr;
		static ClientDownload CLDownload;
		static std::vector<ScriptDownload> ScriptDownloads;

		static void EventHandler(mg_connection *nc, int ev, void *ev_data);
		static void ListHandler(mg_connection *nc, int ev, void *ev_data);
		static void MapHandler(mg_connection *nc, int ev, void *ev_data);
		static void FileHandler(mg_connection *nc, int ev, void *ev_data);
		static void InfoHandler(mg_connection *nc, int ev, void *ev_data);
		static void DownloadHandler(mg_connection *nc, int ev, void *ev_data);

		static bool IsClient(mg_connection *nc);
		static Game::client_t* GetClient(mg_connection *nc);
		static void Forbid(mg_connection *nc);

		static void ModDownloader(ClientDownload* download);
		static bool ParseModList(ClientDownload* download, std::string list);
		static bool DownloadFile(ClientDownload* download, unsigned int index);
	};
}
