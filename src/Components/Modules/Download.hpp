#pragma once
#include <mongoose.h>

namespace Components
{
	class Download : public Component
	{
	public:
		Download();
		~Download();

		void preDestroy() override;

		static void InitiateClientDownload(const std::string& mod, bool needPassword, bool map = false);
		static void InitiateMapDownload(const std::string& map, bool needPassword);

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
			bool isPrivate;
			//mg_mgr mgr;
			Network::Address target;
			std::string hashedPassword;
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
					//mg_mgr_free(&(this->mgr));
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

		static mg_mgr Mgr;
		static ClientDownload CLDownload;
		static std::thread ServerThread;
		static bool Terminate;
		static bool ServerRunning;

		static void DownloadProgress(FileDownload* fDownload, size_t bytes);

		static bool VerifyPassword(mg_connection *nc, http_message* message);

		static void EventHandler(mg_connection *nc, int ev, void *ev_data);
		static void ListHandler(mg_connection *nc, int ev, void *ev_data);
		static void MapHandler(mg_connection *nc, int ev, void *ev_data);
		static void ServerlistHandler(mg_connection *nc, int ev, void *ev_data);
		static void FileHandler(mg_connection *nc, int ev, void *ev_data);
		static void InfoHandler(mg_connection *nc, int ev, void *ev_data);
		static void DownloadHandler(mg_connection *nc, int ev, void *ev_data);

		static bool IsClient(mg_connection *nc);
		static Game::client_t* GetClient(mg_connection *nc);
		static void Forbid(mg_connection *nc);

		static void ModDownloader(ClientDownload* download);
		static bool ParseModList(ClientDownload* download, const std::string& list);
		static bool DownloadFile(ClientDownload* download, unsigned int index);
	};
}
