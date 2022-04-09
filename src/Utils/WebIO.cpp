#include <STDInclude.hpp>
#include <shlwapi.h>

namespace Utils
{
	WebIO::WebIO() : WebIO("WebIO") {}

	WebIO::WebIO(const std::string& useragent, const std::string& url) : WebIO(useragent)
	{
		this->setURL(url);
	}

	WebIO::WebIO(const std::string& useragent) : cancel(false), timeout(5000), hSession(nullptr) // 5 seconds timeout by default
	{
		this->openSession(useragent);
	}

	WebIO::~WebIO()
	{
		this->username.clear();
		this->password.clear();

		this->closeConnection();
		this->closeSession();
	}

	void WebIO::openSession(const std::string& useragent)
	{
		this->closeSession();
		this->hSession = InternetOpenA(useragent.data(), INTERNET_OPEN_TYPE_DIRECT, nullptr, nullptr, 0);
	}

	void WebIO::closeSession()
	{
		if (this->hSession) InternetCloseHandle(this->hSession);
	}

	void WebIO::setCredentials(const std::string& _username, const std::string& _password)
	{
		this->username = _username;
		this->password = _password;
	}

	void WebIO::setURL(std::string _url)
	{
		this->url.server.clear();
		this->url.protocol.clear();
		this->url.document.clear();

		// Insert protocol if none
		if (_url.find("://") == std::string::npos)
		{
			_url = "http://" + _url;
		}

		PARSEDURLA pURL;
		ZeroMemory(&pURL, sizeof(pURL));
		pURL.cbSize = sizeof(pURL);
		ParseURLA(_url.data(), &pURL);

		// Parse protocol
		if (pURL.cchProtocol && pURL.pszProtocol)
		{
			for (UINT i = 0; i < pURL.cchProtocol; ++i)
			{
				char lChar = static_cast<char>(tolower(pURL.pszProtocol[i]));
				this->url.protocol.append(&lChar, 1);
			}
		}
		else
		{
			this->url.protocol.append("http");
		}

		// Parse suffix
		std::string server;

		if (pURL.cchSuffix && pURL.pszSuffix)
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
			this->url.server = server;
			this->url.document = "/";
		}
		else
		{
			this->url.server = server.substr(0, pos);
			this->url.document = server.substr(pos);
		}

		this->url.port.clear();

		pos = this->url.server.find(":");
		if (pos != std::string::npos)
		{
			this->url.port = this->url.server.substr(pos + 1);
			this->url.server = this->url.server.substr(0, pos);
		}

		this->url.raw.clear();
		this->url.raw.append(this->url.protocol);
		this->url.raw.append("://");
		this->url.raw.append(this->url.server);

		if (!this->url.port.empty())
		{
			this->url.raw.append(":");
			this->url.raw.append(this->url.port);
		}

		this->url.raw.append(this->url.document);

		this->isFTP = (this->url.protocol == "ftp");
	}

	std::string WebIO::buildPostBody(WebIO::Params params)
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

	std::string WebIO::postFile(const std::string& _url, const std::string& data, const std::string& fieldName, const std::string& fileName)
	{
		this->setURL(_url);
		return this->postFile(data, fieldName, fileName);
	}

	std::string WebIO::postFile(const std::string& data, std::string fieldName, std::string fileName)
	{
		WebIO::Params headers;

		std::string boundary = "----WebKitFormBoundaryHoLVocRsBxs71fU6";
		headers["Content-Type"] = "multipart/form-data, boundary=" + boundary;

		Utils::String::Replace(fieldName, "\"", "\\\"");
		Utils::String::Replace(fieldName, "\\", "\\\\");
		Utils::String::Replace(fileName, "\"", "\\\"");
		Utils::String::Replace(fileName, "\\", "\\\\");

		std::string body = "--" + boundary + "\r\n";
		body += "Content-Disposition: form-data; name=\"";
		body += fieldName;
		body += "\"; filename=\"";
		body += fileName;
		body += "\"\r\n";
		body += "Content-Type: application/octet-stream\r\n\r\n";
		body += data + "\r\n";
		body += "--" + boundary + "--\r\n";

		headers["Content-Length"] = Utils::String::VA("%u", body.size());

		return this->execute("POST", body, headers);
	}

	std::string WebIO::post(const std::string& _url, const std::string& body, bool* success)
	{
		this->setURL(_url);
		return this->post(body, success);
	}

	std::string WebIO::post(const std::string& _url, WebIO::Params params, bool* success)
	{
		this->setURL(_url);
		return this->post(params, success);
	}

	std::string WebIO::post(WebIO::Params params, bool* success)
	{
		return this->post(this->buildPostBody(params), success);
	}

	std::string WebIO::post(const std::string& body, bool* success)
	{
		return this->execute("POST", body, WebIO::Params(), success);
	}

	std::string WebIO::get(const std::string& _url, bool* success)
	{
		this->setURL(_url);
		return this->get(success);
	}

	std::string WebIO::get(bool* success)
	{
		return this->execute("GET", "", WebIO::Params(), success);
	}

	bool WebIO::openConnection()
	{
		WORD wPort = INTERNET_DEFAULT_HTTP_PORT;
		DWORD dwService = INTERNET_SERVICE_HTTP;
		DWORD dwFlag = 0;

		if (this->isFTP)
		{
			wPort = INTERNET_DEFAULT_FTP_PORT;
			dwService = INTERNET_SERVICE_FTP;
			dwFlag = INTERNET_FLAG_PASSIVE;
		}
		else if (this->isSecuredConnection())
		{
			wPort = INTERNET_DEFAULT_HTTPS_PORT;
		}

		if (!this->url.port.empty())
		{
			wPort = static_cast<WORD>(atoi(this->url.port.data()));
		}

		const char* _username = (this->username.size() ? this->username.data() : NULL);
		const char* _password = (this->password.size() ? this->password.data() : NULL);
		this->hConnect = InternetConnectA(this->hSession, this->url.server.data(), wPort, _username, _password, dwService, dwFlag, 0);

		return (this->hConnect && this->hConnect != INVALID_HANDLE_VALUE);
	}

	void WebIO::closeConnection()
	{
		if (this->hFile && this->hFile != INVALID_HANDLE_VALUE) InternetCloseHandle(this->hFile);
		if (this->hConnect && this->hConnect != INVALID_HANDLE_VALUE) InternetCloseHandle(this->hConnect);
	}

	WebIO* WebIO::setTimeout(DWORD mseconds)
	{
		this->timeout = mseconds;
		return this;
	}

	std::string WebIO::execute(const char* command, const std::string& body, WebIO::Params headers, bool* success)
	{
		if (success) *success = false;
		if (!this->openConnection()) return "";

		const char *acceptTypes[] = { "application/x-www-form-urlencoded", nullptr };

		DWORD dwFlag = INTERNET_FLAG_RELOAD | (this->isSecuredConnection() ? INTERNET_FLAG_SECURE : 0);

		// This doesn't seem to actually do anything, half of those options don't even seem to be implemented.
		// Good job microsoft... ( https://msdn.microsoft.com/en-us/library/windows/desktop/aa385328%28v=vs.85%29.aspx )
		//InternetSetOption(WebIO::m_hConnect, INTERNET_OPTION_CONNECT_TIMEOUT, &m_timeout, sizeof(m_timeout));
		//InternetSetOption(WebIO::m_hConnect, INTERNET_OPTION_RECEIVE_TIMEOUT, &m_timeout, sizeof(m_timeout));
		//InternetSetOption(WebIO::m_hConnect, INTERNET_OPTION_SEND_TIMEOUT, &m_timeout, sizeof(m_timeout));

		this->hFile = HttpOpenRequestA(this->hConnect, command, this->url.document.data(), nullptr, nullptr, acceptTypes, dwFlag, 0);

		if (!this->hFile || this->hFile == INVALID_HANDLE_VALUE)
		{
			this->closeConnection();
			return "";
		}

		if (headers.find("Content-Type") == headers.end())
		{
			headers["Content-Type"] = "application/x-www-form-urlencoded";
		}

		std::string finalHeaders;

		for (auto i = headers.begin(); i != headers.end(); ++i)
		{
			finalHeaders.append(i->first);
			finalHeaders.append(": ");
			finalHeaders.append(i->second);
			finalHeaders.append("\r\n");
		}

		if (HttpSendRequestA(this->hFile, finalHeaders.data(), finalHeaders.size(), const_cast<char*>(body.data()), body.size() + 1) == FALSE)
		{
			return "";
		}

		DWORD statusCode = 404;
		DWORD length = sizeof(statusCode);
		if (HttpQueryInfo(this->hFile, HTTP_QUERY_FLAG_NUMBER | HTTP_QUERY_STATUS_CODE, &statusCode, &length, nullptr) == FALSE || (statusCode != 200 && statusCode != 201))
		{
			this->closeConnection();
			return "";
		}

		DWORD contentLength = 0;
		length = sizeof(statusCode);
		if (HttpQueryInfo(this->hFile, HTTP_QUERY_FLAG_NUMBER | HTTP_QUERY_CONTENT_LENGTH, &contentLength, &length, nullptr) == FALSE)
		{
			contentLength = 0;
		}

		std::string returnBuffer;
		returnBuffer.reserve(contentLength);

		DWORD size = 0;
		char buffer[0x2001] = { 0 };

		while (InternetReadFile(this->hFile, buffer, 0x2000, &size))
		{
			if (this->cancel)
			{
				this->closeConnection();
				return "";
			}

			returnBuffer.append(buffer, size);
			if (this->progressCallback) this->progressCallback(returnBuffer.size(), contentLength);
			if (!size) break;
		}

		this->closeConnection();

		if (success) *success = true;
		return returnBuffer;
	}

	bool WebIO::isSecuredConnection()
	{
		return (this->url.protocol == "https");
	}

	bool WebIO::connect()
	{
		return this->openConnection();
	}

	void WebIO::disconnect()
	{
		this->closeConnection();
	}

	bool WebIO::setDirectory(const std::string& directory)
	{
		return (FtpSetCurrentDirectoryA(this->hConnect, directory.data()) == TRUE);
	}

	bool WebIO::setRelativeDirectory(std::string directory)
	{
		std::string currentDir;

		if (this->getDirectory(currentDir))
		{
			this->formatPath(directory, true);
			this->formatPath(currentDir, true);

			char path[MAX_PATH] = { 0 };
			PathCombineA(path, currentDir.data(), directory.data());

			std::string newPath(path);
			this->formatPath(newPath, false);

			return this->setDirectory(newPath);
		}

		return false;
	}

	bool WebIO::getDirectory(std::string &directory)
	{
		directory.clear();

		DWORD size = MAX_PATH;
		char currentDir[MAX_PATH] = { 0 };

		if (FtpGetCurrentDirectoryA(this->hConnect, currentDir, &size) == TRUE)
		{
			directory.append(currentDir, size);
			return true;
		}

		return false;
	}

	void WebIO::formatPath(std::string& path, bool win)
	{
		size_t nPos;
		std::string find = "\\";
		std::string replace = "/";

		if (win)
		{
			find = "/";
			replace = "\\";
		}

		while ((nPos = path.find(find)) != std::string::npos)
		{
			path = path.replace(nPos, find.length(), replace);
		}
	}

	bool WebIO::createDirectory(const std::string& directory)
	{
		return (FtpCreateDirectoryA(this->hConnect, directory.data()) == TRUE);
	}

	// Recursively delete a directory
	bool WebIO::deleteDirectory(const std::string& directory)
	{
		std::string tempDir;
		this->getDirectory(tempDir);

		this->setRelativeDirectory(directory);

		std::vector<std::string> list;

		this->listFiles(".", list);
		for (auto file : list) this->deleteFile(file);

		this->listDirectories(".", list);
		for (auto& dir : list) this->deleteDirectory(dir);

		this->setDirectory(tempDir);

		return (FtpRemoveDirectoryA(this->hConnect, directory.data()) == TRUE);
	}

	bool WebIO::renameDirectory(const std::string& directory, const std::string& newDir)
	{
		return (FtpRenameFileA(this->hConnect, directory.data(), newDir.data()) == TRUE); // According to the internetz, this should work
	}

	bool WebIO::listElements(const std::string& directory, std::vector<std::string>& list, bool files)
	{
		list.clear();

		WIN32_FIND_DATAA findFileData;
		bool result = false;
		DWORD dwAttribute = (files ? FILE_ATTRIBUTE_NORMAL : FILE_ATTRIBUTE_DIRECTORY);

		// Any filename.
		std::string tempDir;
		this->getDirectory(tempDir);
		this->setRelativeDirectory(directory);

		this->hFile = FtpFindFirstFileA(this->hConnect, "*", &findFileData, INTERNET_FLAG_RELOAD, NULL);

		if (this->hFile && this->hFile != INVALID_HANDLE_VALUE)
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
			} while (InternetFindNextFileA(this->hFile, &findFileData));

			InternetCloseHandle(this->hFile);
		}

		this->setDirectory(tempDir);

		return result;
	}

	bool WebIO::listDirectories(const std::string& directory, std::vector<std::string>& list)
	{
		return this->listElements(directory, list, false);
	}

	bool WebIO::listFiles(const std::string& directory, std::vector<std::string>& list)
	{
		return this->listElements(directory, list, true);
	}

	bool WebIO::uploadFile(const std::string& file, const std::string& localfile)
	{
		return (FtpPutFileA(this->hConnect, localfile.data(), file.data(), FTP_TRANSFER_TYPE_BINARY, NULL) == TRUE);
	}

	bool WebIO::deleteFile(const std::string& file)
	{
		return (FtpDeleteFileA(this->hConnect, file.data()) == TRUE);
	}

	bool WebIO::renameFile(const std::string& file, const std::string& newFile)
	{
		return (FtpRenameFileA(this->hConnect, file.data(), newFile.data()) == TRUE);
	}

	bool WebIO::downloadFile(const std::string& file, const std::string& localfile)
	{
		return (FtpGetFileA(this->hConnect, file.data(), localfile.data(), FALSE, NULL, FTP_TRANSFER_TYPE_BINARY, 0) == TRUE);
	}

	bool WebIO::uploadFileData(const std::string& file, const std::string& data)
	{
		bool result = false;
		this->hFile = FtpOpenFileA(this->hConnect, file.data(), GENERIC_WRITE, INTERNET_FLAG_TRANSFER_BINARY | INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_RELOAD, 0);

		if (this->hFile)
		{
			DWORD size = 0;
			if (InternetWriteFile(this->hFile, data.data(), data.size(), &size) == TRUE)
			{
				result = (size == data.size());
			}

			InternetCloseHandle(this->hFile);
		}

		return result;
	}

	bool WebIO::downloadFileData(const std::string& file, std::string& data)
	{
		data.clear();

		this->hFile = FtpOpenFileA(this->hConnect, file.data(), GENERIC_READ, INTERNET_FLAG_TRANSFER_BINARY | INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_RELOAD, 0);

		if (this->hFile)
		{
			DWORD size = 0;
			char buffer[0x2001] = { 0 };

			while (InternetReadFile(this->hFile, buffer, 0x2000, &size))
			{
				data.append(buffer, size);
				if (!size) break;
			}

			InternetCloseHandle(this->hFile);
			return true;
		}

		return false;
	}

	void WebIO::setProgressCallback(Utils::Slot<void(size_t, size_t)> callback)
	{
		this->progressCallback = callback;
	}
}
