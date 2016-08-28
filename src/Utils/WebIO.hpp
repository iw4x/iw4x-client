/*
	This project is released under the GPL 2.0 license.
	Some parts are based on research by Bas Timmer and the OpenSteamworks project.
	Please do no evil.

	Initial author: (https://github.com/)momo5502
	Started: 2015-03-01
	Notes:
		Small FTP and HTTP utility class using WinAPI
*/

namespace Utils
{
	class WebIO
	{
	public:

		typedef std::map<std::string, std::string> Params;

		WebIO();
		WebIO(std::string useragent);
		WebIO(std::string useragent, std::string url);

		~WebIO();

		void SetURL(std::string url);
		void SetCredentials(std::string username, std::string password);

		std::string PostFile(std::string url, std::string data, std::string fieldName, std::string fileName);
		std::string PostFile(std::string data, std::string fieldName, std::string fileName);

		std::string Post(std::string url, WebIO::Params params);
		std::string Post(std::string url, std::string body);
		std::string Post(WebIO::Params params);
		std::string Post(std::string body);

		std::string Get(std::string url);
		std::string Get();

		WebIO* SetTimeout(DWORD mseconds);

		// FTP
		bool Connect();
		void Disconnect(); // Not necessary

		bool SetDirectory(std::string directory);
		bool SetRelativeDirectory(std::string directory);
		bool GetDirectory(std::string &directory);
		bool CreateDirectory(std::string directory);
		bool DeleteDirectory(std::string directory);
		bool RenameDirectory(std::string directory, std::string newDir);

		bool ListDirectories(std::string directory, std::vector<std::string> &list);
		bool ListFiles(std::string directory, std::vector<std::string> &list);

		bool DeleteFile(std::string file);
		bool RenameFile(std::string file, std::string newFile);
		bool UploadFile(std::string file, std::string localfile);
		bool DownloadFile(std::string file, std::string localfile);

		bool UploadFileData(std::string file, std::string data);
		bool DownloadFileData(std::string file, std::string &data);

	private:

		enum Command
		{
			COMMAND_POST,
			COMMAND_GET,
		};

		struct WebURL
		{
			std::string protocol;
			std::string server;
			std::string document;
			std::string raw;
		};

		bool m_isFTP;
		std::string m_username;
		std::string m_password;

		WebURL m_sUrl;

		HINTERNET m_hSession;
		HINTERNET m_hConnect;
		HINTERNET m_hFile;

		DWORD m_timeout;

		std::string BuildPostBody(WebIO::Params params);

		bool IsSecuredConnection();

		std::string Execute(const char* command, std::string body, WebIO::Params headers = WebIO::Params());

		bool ListElements(std::string directory, std::vector<std::string> &list, bool files);

		void OpenSession(std::string useragent);
		void CloseSession();

		bool OpenConnection();
		void CloseConnection();

		void FormatPath(std::string &path, bool win); /* if (win == true):  / -> \\ */
	};
}
