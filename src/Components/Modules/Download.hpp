namespace Components
{
	class Download : public Component
	{
	public:
		Download();
		~Download();

#ifdef DEBUG
		const char* GetName() { return "Download"; };
#endif

		static void InitiateClientDownload(std::string mod);

	private:
		class ClientDownload
		{
		public:
			ClientDownload() : Valid(false), Running(false), TerminateThread(false), TotalBytes(0), DownBytes(0), LastTimeStamp(0), TimeStampBytes(0) {}
			~ClientDownload() { this->Clear(); }

			bool Running;
			bool Valid;
			bool TerminateThread;
			mg_mgr Mgr;
			Network::Address Target;
			std::string Mod;
			std::thread Thread;

			size_t TotalBytes;
			size_t DownBytes;

			int LastTimeStamp;
			size_t TimeStampBytes;

			class File
			{
			public:
				std::string Name;
				std::string Hash;
				size_t Size;
			};

			std::vector<File> Files;

			void Clear()
			{
				this->TerminateThread = true;

				if (this->Thread.joinable())
				{
					this->Thread.join();
				}

				this->Running = false;
				this->Mod.clear();
				this->Files.clear();

				if (this->Valid)
				{
					this->Valid = false;
					mg_mgr_free(&(this->Mgr));
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
