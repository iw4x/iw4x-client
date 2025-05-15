#pragma once


struct mg_connection;
struct mg_http_message;

namespace Components
{
	class Download : public Component
	{
	public:
		Download();
		~Download();

		void preDestroy() override;

		static void InitiateClientDownload(const std::string& mod, bool needPassword, bool map = false, bool downloadOnly = false);
		static void InitiateMapDownload(const std::string& map, bool needPassword);

		static void ReplyError(mg_connection* connection, int code, std::string messageOverride = {});

		static Dvar::Var SV_wwwDownload;
		static Dvar::Var SV_wwwBaseUrl;

		static Dvar::Var UIDlTimeLeft;
		static Dvar::Var UIDlProgress;
		static Dvar::Var UIDlTransRate;

	private:
		class ClientDownload
		{
		public:
			ClientDownload(bool isMap = false, bool downloadOnly = false) : running_(false), valid_(false), terminateThread_(false), isMap_(isMap), downloadOnly_(downloadOnly), totalBytes_(0), downBytes_(0), lastTimeStamp_(0), timeStampBytes_(0) {}
			~ClientDownload() { this->clear(); }

			bool running_;
			bool valid_;
			bool terminateThread_;
			bool downloadOnly_;
			bool isMap_;
			bool isPrivate_;
			Network::Address target_;
			std::string hashedPassword_;
			std::string mod_;
			std::thread thread_;

			std::size_t totalBytes_;
			std::size_t downBytes_;

			int lastTimeStamp_;
			std::size_t timeStampBytes_;

			class File
			{
			public:
				std::string name;
				std::string hash;
				std::size_t size;
				bool isMap;

				bool allowed() const;
			};

			std::vector<File> files_;

			void clear()
			{
				this->terminateThread_ = true;

				if (this->thread_.joinable())
				{
					this->thread_.join();
				}

				this->running_ = false;
				this->mod_.clear();
				this->files_.clear();

				if (this->valid_)
				{
					this->valid_ = false;
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
		static void Reply(mg_connection* connection, const std::string& contentType, const std::string& data);

		static std::optional<std::string> FileHandler(mg_connection* c, const mg_http_message* hm);
		static void EventHandler(mg_connection* c, const int ev, void* ev_data, void* fn_data);
		static std::optional<std::string> ListHandler(mg_connection* c, const mg_http_message* hm);
		static std::optional<std::string> InfoHandler(mg_connection* c, const mg_http_message* hm);
		static std::optional<std::string> ServerListHandler(mg_connection* c, const mg_http_message* hm);
		static std::optional<std::string> MapHandler(mg_connection* c, const mg_http_message* hm);
	};
}
