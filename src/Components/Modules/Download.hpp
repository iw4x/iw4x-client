#pragma once
#include "Game/Functions.hpp"

namespace Components
{
	class Download : public Component
	{
	public:
		Download();
		~Download();

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
			ScriptDownload(std::string _url, unsigned int _object) : url(_url), object(_object), webIO(nullptr), done(false), notifyRequired(false), totalSize(0), currentSize(0)
			{
				Game::AddRefToObject(this->getObject());
			}

			ScriptDownload(ScriptDownload&& other) noexcept = delete;
			ScriptDownload& operator=(ScriptDownload&& other) noexcept = delete;

			~ScriptDownload()
			{
				if (this->getObject())
				{
					Game::RemoveRefToObject(this->getObject());
					this->object = 0;
				}

				if (this->workerThread.joinable())
				{
					this->workerThread.join();
				}

				this->destroyWebIO();
			}

			void startWorking()
			{
				if (!this->isWorking())
				{
					this->workerThread = std::thread(std::bind(&ScriptDownload::handler, this));
				}
			}

			bool isWorking()
			{
				return this->workerThread.joinable();
			}

			void notifyProgress()
			{
				if (this->notifyRequired)
				{
					this->notifyRequired = false;

					if (Game::Scr_IsSystemActive())
					{
						Game::Scr_AddInt(static_cast<int>(this->totalSize));
						Game::Scr_AddInt(static_cast<int>(this->currentSize));
						Game::Scr_NotifyId(this->getObject(), Game::SL_GetString("progress", 0), 2);
					}
				}
			}

			void updateProgress(size_t _currentSize, size_t _toalSize)
			{
				this->currentSize = _currentSize;
				this->totalSize = _toalSize;
				this->notifyRequired = true;
			}

			void notifyDone()
			{
				if (!this->isDone()) return;

				if (Game::Scr_IsSystemActive())
				{
					Game::Scr_AddString(this->result.data()); // No binary data supported yet
					Game::Scr_AddInt(this->success);
					Game::Scr_NotifyId(this->getObject(), Game::SL_GetString("done", 0), 2);
				}
			}

			bool isDone() { return this->done; };

			std::string getUrl() { return this->url; }
			unsigned int getObject() { return this->object; }

			void cancel()
			{
				if (this->webIO)
				{
					this->webIO->cancelDownload();
				}
			}

		private:
			std::string url;
			std::string result;
			unsigned int object;
			std::thread workerThread;
			Utils::WebIO* webIO;

			bool done;
			bool success;
			bool notifyRequired;
			size_t totalSize;
			size_t currentSize;

			void handler()
			{
				this->destroyWebIO();

				this->webIO = new Utils::WebIO("IW4x");
				this->webIO->setProgressCallback(std::bind(&ScriptDownload::updateProgress, this, std::placeholders::_1, std::placeholders::_2));

				this->result = this->webIO->get(this->url, &this->success);

				this->destroyWebIO();
				this->done = true;
			}

			void destroyWebIO()
			{
				if (this->webIO)
				{
					delete this->webIO;
					this->webIO = nullptr;
				}
			}
		};

		static mg_mgr Mgr;
		static ClientDownload CLDownload;
		static std::vector<std::shared_ptr<ScriptDownload>> ScriptDownloads;
		static std::thread ServerThread;
		static bool Terminate;

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
