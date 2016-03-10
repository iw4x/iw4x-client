#include "STDInclude.hpp"

namespace Utils
{
	WebIO::WebIO() : WebIO("WebIO") {}

	WebIO::WebIO(std::string useragent, std::string url) : WebIO(useragent)
	{
		WebIO::SetURL(url);
	}

	WebIO::WebIO(std::string useragent) : m_timeout(5000) // 5 seconds timeout by default
	{
		WebIO::OpenSession(useragent);
	}

	WebIO::~WebIO()
	{
		WebIO::m_username.clear();
		WebIO::m_password.clear();

		WebIO::CloseConnection();
		WebIO::CloseSession();
	}

	void WebIO::OpenSession(std::string useragent)
	{
		WebIO::m_hSession = InternetOpenA(useragent.data(), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	}

	void WebIO::CloseSession()
	{
		InternetCloseHandle(WebIO::m_hSession);
	}

	void WebIO::SetCredentials(std::string username, std::string password)
	{
		WebIO::m_username.clear();
		WebIO::m_password.clear();

		WebIO::m_username.append(username.begin(), username.end());
		WebIO::m_password.append(password.begin(), password.end());
	}

	void WebIO::SetURL(std::string url)
	{
		WebIO::m_sUrl.server.clear();
		WebIO::m_sUrl.protocol.clear();
		WebIO::m_sUrl.document.clear();

		// Insert protocol if none
		if (url.find("://") == std::string::npos)
		{
			url = "http://" + url;
		}

		PARSEDURLA pURL;
		pURL.cbSize = sizeof(pURL);
		ParseURLA(url.data(), &pURL);

		// Parse protocol
		if (pURL.cchProtocol && pURL.cchProtocol != 0xCCCCCCCC && pURL.pszProtocol)
		{
			for (UINT i = 0; i < pURL.cchProtocol; ++i)
			{
				char lChar = static_cast<char>(tolower(pURL.pszProtocol[i]));
				WebIO::m_sUrl.protocol.append(&lChar, 1);
			}
		}
		else
		{
			WebIO::m_sUrl.protocol.append("http");
		}

		// Parse suffix
		std::string server;

		if (pURL.cchSuffix && pURL.cchSuffix != 0xCCCCCCCC && pURL.pszSuffix)
		{
			server.append(pURL.pszSuffix, pURL.cchSuffix);
		}
		else
		{
			// TODO: Add some error handling here
			return;
		}

		// Remove '//' from the url
		if (!server.find("//"))
		{
			server = server.substr(2);
		}

		size_t pos = server.find("/");
		if (pos == std::string::npos)
		{
			WebIO::m_sUrl.server = server;
			WebIO::m_sUrl.document = "/";
		}
		else
		{
			WebIO::m_sUrl.server = server.substr(0, pos);
			WebIO::m_sUrl.document = server.substr(pos);
		}

		WebIO::m_sUrl.raw.clear();
		WebIO::m_sUrl.raw.append(WebIO::m_sUrl.protocol);
		WebIO::m_sUrl.raw.append("://");
		WebIO::m_sUrl.raw.append(WebIO::m_sUrl.server);
		WebIO::m_sUrl.raw.append(WebIO::m_sUrl.document);

		WebIO::m_isFTP = (WebIO::m_sUrl.protocol == "ftp");
	}

	std::string WebIO::BuildPostBody(WebIO::Params params)
	{
		std::string body;

		for (auto param = params.begin(); param != params.end(); ++param)
		{
			std::string key = param->first;
			std::string value = param->second;

			if (!body.empty()) body.append("&");

			body.append(key);
			body.append("=");
			body.append(value);
		}

		body.append("\0");

		return body;
	}

	std::string WebIO::Post(std::string url, std::string body)
	{
		WebIO::SetURL(url);
		return WebIO::Post(body);
	}

	std::string WebIO::Post(std::string url, WebIO::Params params)
	{
		WebIO::SetURL(url);
		return WebIO::Post(params);
	}

	std::string WebIO::Post(WebIO::Params params)
	{
		return WebIO::Post(WebIO::BuildPostBody(params));
	}

	std::string WebIO::Post(std::string body)
	{
		return WebIO::Execute("POST", body);
	}

	std::string WebIO::Get(std::string url)
	{
		WebIO::SetURL(url);
		return WebIO::Get();
	}

	std::string WebIO::Get()
	{
		return WebIO::Execute("GET", "");
	}

	bool WebIO::OpenConnection()
	{
		WORD wPort = INTERNET_DEFAULT_HTTP_PORT;
		DWORD dwService = INTERNET_SERVICE_HTTP;
		DWORD dwFlag = 0;

		if (WebIO::m_isFTP)
		{
			wPort = INTERNET_DEFAULT_FTP_PORT;
			dwService = INTERNET_SERVICE_FTP;
			dwFlag = INTERNET_FLAG_PASSIVE;
		}
		else if (WebIO::IsSecuredConnection())
		{
			wPort = INTERNET_DEFAULT_HTTPS_PORT;
		}

		const char* username = (WebIO::m_username.size() ? WebIO::m_username.data() : NULL);
		const char* password = (WebIO::m_password.size() ? WebIO::m_password.data() : NULL);
		WebIO::m_hConnect = InternetConnectA(WebIO::m_hSession, WebIO::m_sUrl.server.data(), wPort, username, password, dwService, dwFlag, 0);

		return (WebIO::m_hConnect && WebIO::m_hConnect != INVALID_HANDLE_VALUE);
	}

	void WebIO::CloseConnection()
	{
		if (WebIO::m_hFile && WebIO::m_hFile != INVALID_HANDLE_VALUE) InternetCloseHandle(WebIO::m_hFile);
		if (WebIO::m_hConnect && WebIO::m_hConnect != INVALID_HANDLE_VALUE) InternetCloseHandle(WebIO::m_hConnect);
	}

	WebIO* WebIO::SetTimeout(DWORD mseconds)
	{
		this->m_timeout = mseconds;
		return this;
	}

	std::string WebIO::Execute(const char* command, std::string body)
	{
		if (!WebIO::OpenConnection()) return "";

		const char *acceptTypes[] = { "application/x-www-form-urlencoded", nullptr };

		DWORD dwFlag = INTERNET_FLAG_RELOAD | (WebIO::IsSecuredConnection() ? INTERNET_FLAG_SECURE : 0);

		// This doesn't seem to actually do anything, half of those options don't even seem to be implemented.
		// Good job microsoft... ( https://msdn.microsoft.com/en-us/library/windows/desktop/aa385328%28v=vs.85%29.aspx )
		//InternetSetOption(WebIO::m_hConnect, INTERNET_OPTION_CONNECT_TIMEOUT, &m_timeout, sizeof(m_timeout));
		//InternetSetOption(WebIO::m_hConnect, INTERNET_OPTION_RECEIVE_TIMEOUT, &m_timeout, sizeof(m_timeout));
		//InternetSetOption(WebIO::m_hConnect, INTERNET_OPTION_SEND_TIMEOUT, &m_timeout, sizeof(m_timeout));

		WebIO::m_hFile = HttpOpenRequestA(WebIO::m_hConnect, command, WebIO::m_sUrl.document.data(), NULL, NULL, acceptTypes, dwFlag, 0);

		if (!WebIO::m_hFile || WebIO::m_hFile == INVALID_HANDLE_VALUE)
		{
			WebIO::CloseConnection();
			return "";
		}

		const char* headers = "Content-type: application/x-www-form-urlencoded";
		HttpSendRequestA(WebIO::m_hFile, headers, strlen(headers), const_cast<char*>(body.data()), body.size() + 1);

		std::string returnBuffer;

		DWORD size = 0;
		char buffer[0x2001] = { 0 };

		while (InternetReadFile(WebIO::m_hFile, buffer, 0x2000, &size))
		{
			returnBuffer.append(buffer, size);
			if (!size) break;
		}

		WebIO::CloseConnection();

		return returnBuffer;
	}

	bool WebIO::IsSecuredConnection()
	{
		return (WebIO::m_sUrl.protocol == "https");
	}

	bool WebIO::Connect()
	{
		return WebIO::OpenConnection();
	}

	void WebIO::Disconnect()
	{
		WebIO::CloseConnection();
	}

	bool WebIO::SetDirectory(std::string directory)
	{
		return (FtpSetCurrentDirectoryA(WebIO::m_hConnect, directory.data()) == TRUE);
	}

	bool WebIO::SetRelativeDirectory(std::string directory)
	{
		std::string currentDir;

		if (WebIO::GetDirectory(currentDir))
		{
			WebIO::FormatPath(directory, true);
			WebIO::FormatPath(currentDir, true);

			char path[MAX_PATH] = { 0 };
			PathCombineA(path, currentDir.data(), directory.data());

			std::string newPath(path);
			WebIO::FormatPath(newPath, false);

			return WebIO::SetDirectory(newPath);
		}

		return false;
	}

	bool WebIO::GetDirectory(std::string &directory)
	{
		directory.clear();

		DWORD size = MAX_PATH;
		char currentDir[MAX_PATH] = { 0 };

		if (FtpGetCurrentDirectoryA(WebIO::m_hConnect, currentDir, &size) == TRUE)
		{
			directory.append(currentDir, size);
			return true;
		}

		return false;
	}

	void WebIO::FormatPath(std::string &path, bool win)
	{
		size_t nPos;
		std::string find = "\\";
		std::string replace = "/";

		if (win)
		{
			find = "/";
			replace = "\\";
		}

		while ((nPos = path.find(find)) != std::wstring::npos)
		{
			path = path.replace(nPos, find.length(), replace);
		}
	}

	bool WebIO::CreateDirectory(std::string directory)
	{
		return (FtpCreateDirectoryA(WebIO::m_hConnect, directory.data()) == TRUE);
	}

	// Recursively delete a directory
	bool WebIO::DeleteDirectory(std::string directory)
	{
		std::string tempDir;
		WebIO::GetDirectory(tempDir);

		WebIO::SetRelativeDirectory(directory);

		std::vector<std::string> list;

		WebIO::ListFiles(".", list);
		for (auto file : list) WebIO::DeleteFile(file);

		WebIO::ListDirectories(".", list);
		for (auto dir : list) WebIO::DeleteDirectory(dir);

		WebIO::SetDirectory(tempDir);

		return (FtpRemoveDirectoryA(WebIO::m_hConnect, directory.data()) == TRUE);
	}

	bool WebIO::RenameDirectory(std::string directory, std::string newDir)
	{
		return (FtpRenameFileA(WebIO::m_hConnect, directory.data(), newDir.data()) == TRUE); // According to the internetz, this should work
	}

	bool WebIO::ListElements(std::string directory, std::vector<std::string> &list, bool files)
	{
		list.clear();

		WIN32_FIND_DATAA findFileData;
		bool result = false;
		DWORD dwAttribute = (files ? FILE_ATTRIBUTE_NORMAL : FILE_ATTRIBUTE_DIRECTORY);

		// Any filename.
		std::string tempDir;
		WebIO::GetDirectory(tempDir);
		WebIO::SetRelativeDirectory(directory);

		WebIO::m_hFile = FtpFindFirstFileA(WebIO::m_hConnect, "*", &findFileData, INTERNET_FLAG_RELOAD, NULL);

		if (WebIO::m_hFile != INVALID_HANDLE_VALUE)
		{
			do
			{
				//if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_OFFLINE) continue;
				//if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) continue;

				if (findFileData.dwFileAttributes == dwAttribute) // No bitwise flag check, as it might return archives/offline/hidden or other files/dirs
				{
					//printf("%s: %X\n", findFileData.cFileName, findFileData.dwFileAttributes);
					list.push_back(findFileData.cFileName);
					result = true;
				}
			} while (InternetFindNextFileA(WebIO::m_hFile, &findFileData));

			InternetCloseHandle(WebIO::m_hFile);
		}

		WebIO::SetDirectory(tempDir);

		return result;
	}

	bool WebIO::ListDirectories(std::string directory, std::vector<std::string> &list)
	{
		return WebIO::ListElements(directory, list, false);
	}

	bool WebIO::ListFiles(std::string directory, std::vector<std::string> &list)
	{
		return WebIO::ListElements(directory, list, true);
	}

	bool WebIO::UploadFile(std::string file, std::string localfile)
	{
		return (FtpPutFileA(WebIO::m_hConnect, localfile.data(), file.data(), FTP_TRANSFER_TYPE_BINARY, NULL) == TRUE);
	}

	bool WebIO::DeleteFile(std::string file)
	{
		return (FtpDeleteFileA(WebIO::m_hConnect, file.data()) == TRUE);
	}

	bool WebIO::RenameFile(std::string file, std::string newFile)
	{
		return (FtpRenameFileA(WebIO::m_hConnect, file.data(), newFile.data()) == TRUE);
	}

	bool WebIO::DownloadFile(std::string file, std::string localfile)
	{
		return (FtpGetFileA(WebIO::m_hConnect, file.data(), localfile.data(), FALSE, NULL, FTP_TRANSFER_TYPE_BINARY, 0) == TRUE);
	}

	bool WebIO::UploadFileData(std::string file, std::string data)
	{
		bool result = false;
		WebIO::m_hFile = FtpOpenFileA(WebIO::m_hConnect, file.data(), GENERIC_WRITE, INTERNET_FLAG_TRANSFER_BINARY | INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_RELOAD, 0);

		if (WebIO::m_hFile)
		{
			DWORD size = 0;
			if (InternetWriteFile(WebIO::m_hFile, data.data(), data.size(), &size) == TRUE)
			{
				result = (size == data.size());
			}

			InternetCloseHandle(WebIO::m_hFile);
		}

		return result;
	}

	bool WebIO::DownloadFileData(std::string file, std::string &data)
	{
		data.clear();

		WebIO::m_hFile = FtpOpenFileA(WebIO::m_hConnect, file.data(), GENERIC_READ, INTERNET_FLAG_TRANSFER_BINARY | INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_RELOAD, 0);

		if (WebIO::m_hFile)
		{
			DWORD size = 0;
			char buffer[0x2001] = { 0 };

			while (InternetReadFile(WebIO::m_hFile, buffer, 0x2000, &size))
			{
				data.append(buffer, size);
				if (!size) break;
			}

			InternetCloseHandle(WebIO::m_hFile);
			return true;
		}

		return false;
	}
}
