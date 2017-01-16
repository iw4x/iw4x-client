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

		void setURL(std::string url);
		void SetCredentials(std::string username, std::string password);

		std::string postFile(std::string url, std::string data, std::string fieldName, std::string fileName);
		std::string postFile(std::string data, std::string fieldName, std::string fileName);

		std::string post(std::string url, WebIO::Params params);
		std::string post(std::string url, std::string body);
		std::string post(WebIO::Params params);
		std::string post(std::string body);

		std::string get(std::string url);
		std::string get();

		WebIO* setTimeout(DWORD mseconds);

		// FTP
		bool connect();
		void disconnect(); // Not necessary

		bool setDirectory(std::string directory);
		bool setRelativeDirectory(std::string directory);
		bool getDirectory(std::string &directory);
		bool createDirectory(std::string directory);
		bool deleteDirectory(std::string directory);
		bool renameDirectory(std::string directory, std::string newDir);

		bool listDirectories(std::string directory, std::vector<std::string> &list);
		bool listFiles(std::string directory, std::vector<std::string> &list);

		bool deleteFile(std::string file);
		bool renameFile(std::string file, std::string newFile);
		bool uploadFile(std::string file, std::string localfile);
		bool downloadFile(std::string file, std::string localfile);

		bool uploadFileData(std::string file, std::string data);
		bool downloadFileData(std::string file, std::string &data);

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
			std::string port;
			std::string raw;
		};

		bool isFTP;
		std::string username;
		std::string password;

		WebURL url;

		HINTERNET hSession;
		HINTERNET hConnect;
		HINTERNET hFile;

		DWORD timeout;

		std::string buildPostBody(WebIO::Params params);

		bool isSecuredConnection();

		std::string execute(const char* command, std::string body, WebIO::Params headers = WebIO::Params());

		bool listElements(std::string directory, std::vector<std::string> &list, bool files);

		void openSession(std::string useragent);
		void closeSession();

		bool openConnection();
		void closeConnection();

		void formatPath(std::string &path, bool win); /* if (win == true):  / -> \\ */
	};
}
