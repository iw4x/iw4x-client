#pragma once

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

		static Dvar::Var SV_wwwDownload;
		static Dvar::Var SV_wwwBaseUrl;

		static Dvar::Var UIDlTimeLeft;
		static Dvar::Var UIDlProgress;
		static Dvar::Var UIDlTransRate;

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
			Network::Address target;
			std::string hashedPassword;
			std::string mod;
			std::thread thread;

			std::size_t totalBytes;
			std::size_t downBytes;

			int lastTimeStamp;
			std::size_t timeStampBytes;

			class File
			{
			public:
				std::string name;
				std::string hash;
				std::size_t size;
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
			std::size_t receivedBytes;
		};

		static ClientDownload CLDownload;
		static std::thread ServerThread;
		static volatile bool Terminate;
		static bool ServerRunning;

		static std::string MongooseLogBuffer;

		static void DownloadProgress(FileDownload* fDownload, std::size_t bytes);

		static void ModDownloader(ClientDownload* download);
		static bool ParseModList(ClientDownload* download, const std::string& list);
		static bool DownloadFile(ClientDownload* download, unsigned int index);

		static void LogFn(char c, void* param);
	};
}
