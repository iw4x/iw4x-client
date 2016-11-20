namespace Components
{
	class Download : public Component
	{
	public:
		Download();
		~Download();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* getName() { return "Download"; };
#endif

		static void InitiateClientDownload(std::string mod);

	private:
		class ClientDownload
		{
		public:
			ClientDownload() : valid(false), running(false), terminateThread(false), totalBytes(0), downBytes(0), lastTimeStamp(0), timeStampBytes(0) {}
			~ClientDownload() { this->clear(); }

			bool running;
			bool valid;
			bool terminateThread;
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

		static mg_mgr Mgr;
		static ClientDownload CLDownload;

		static void EventHandler(mg_connection *nc, int ev, void *ev_data);
		static void ListHandler(mg_connection *nc, int ev, void *ev_data);
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
