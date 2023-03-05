/*
	This project is released under the GPL 2.0 license.
	Some parts are based on research by Bas Timmer and the OpenSteamworks project.
	Please do no evil.

	Initial author: (https://github.com/)momo5502
	Started: 2015-03-01
	Notes:
		Small FTP and HTTP utility class using WinAPI
*/

#pragma once

namespace Utils
{
	class WebIO
	{
	public:
		using params = std::map<std::string, std::string>;

		WebIO();
		WebIO(const std::string& useragent);
		WebIO(const std::string& useragent, const std::string& url);

		~WebIO();

		void setURL(std::string url);
		void setCredentials(const std::string& username, const std::string& password);

		std::string postFile(const std::string& url, const std::string& data, const std::string& fieldName, const std::string& fileName);
		std::string postFile(const std::string& data, std::string fieldName, std::string fileName);

		std::string post(const std::string& url, const params& params, bool* success = nullptr);
		std::string post(const std::string& url, const std::string& body, bool* success = nullptr);
		std::string post(const params& params, bool* success = nullptr);
		std::string post(const std::string& body, bool* success = nullptr);

		std::string get(const std::string& url, bool* success = nullptr);
		std::string get(bool* success = nullptr);

		WebIO* setTimeout(DWORD msec);

		// FTP
		bool connect();
		void disconnect(); // Not necessary

		bool setDirectory(const std::string&directory);
		bool setRelativeDirectory(std::string directory);
		bool getDirectory(std::string* directory) const;
		bool createDirectory(const std::string& directory) const;
		bool deleteDirectory(const std::string& directory);
		bool renameDirectory(const std::string& directory, const std::string& newDir) const;

		bool listDirectories(const std::string& directory, std::vector<std::string>& list);
		bool listFiles(const std::string& directory, std::vector<std::string>& list);

		bool deleteFile(const std::string& file) const;
		bool renameFile(const std::string& file, const std::string& newFile) const;
		bool downloadFile(const std::string& file, const std::string& localFile) const;
		bool uploadFile(const std::string& file, const std::string& localFile) const;

		bool uploadFileData(const std::string& file,const std::string& data);
		bool downloadFileData(const std::string& file, std::string& data);

		void setProgressCallback(const Slot<void(std::size_t, std::size_t)>& callback);
		void cancelDownload();

		static std::string GetCacheBuster();

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

		bool cancel_;

		bool isFTP_;
		std::string username_;
		std::string password_;

		WebURL url_;

		HINTERNET hSession_;
		HINTERNET hConnect_;
		HINTERNET hFile_;

		DWORD timeout_;

		Slot<void(size_t, size_t)> progressCallback;

		static std::string buildPostBody(const params& params);

		[[nodiscard]] bool isSecuredConnection() const;

		std::string execute(const char* command, const std::string& body, const params& headers, bool* success = nullptr);

		bool listElements(const std::string& directory, std::vector<std::string>& list, bool files);

		void openSession(const std::string& useragent);
		void closeSession();

		bool openConnection();
		void closeConnection();

		static void FormatPath(std::string& path, bool win); /* if (win == true):  / -> \\ */
	};
}
