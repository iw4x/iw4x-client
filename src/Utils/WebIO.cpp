#include <STDInclude.hpp>
#include <Shlwapi.h>

#include "WebIO.hpp"

namespace Utils
{
	WebIO::WebIO() : WebIO("WebIO") {}

	WebIO::WebIO(const std::string& useragent, const std::string& url) : WebIO(useragent)
	{
		this->setURL(url);
	}

	WebIO::WebIO(const std::string& useragent) : cancel_(false), hSession_(nullptr), timeout_(5000)  // 5 seconds timeout by default
	{
		this->openSession(useragent);
	}

	WebIO::~WebIO()
	{
		this->closeConnection();
		this->closeSession();
	}

	void WebIO::openSession(const std::string& useragent)
	{
		this->closeSession();
		this->hSession_ = InternetOpenA(useragent.data(), INTERNET_OPEN_TYPE_DIRECT, nullptr, nullptr, 0);
	}

	void WebIO::closeSession()
	{
		if (this->hSession_) InternetCloseHandle(this->hSession_);
	}

	void WebIO::setCredentials(const std::string& username, const std::string& password)
	{
		this->username_ = username;
		this->password_ = password;
	}

	void WebIO::setURL(std::string url)
	{
		this->url_.server.clear();
		this->url_.protocol.clear();
		this->url_.document.clear();

		// Insert protocol if none
		if (url.find("://") == std::string::npos)
		{
			url = "http://" + url;
		}

		PARSEDURLA pURL;
		ZeroMemory(&pURL, sizeof(pURL));
		pURL.cbSize = sizeof(pURL);
		ParseURLA(url.data(), &pURL);

		// Parse protocol
		if (pURL.cchProtocol && pURL.pszProtocol)
		{
			for (UINT i = 0; i < pURL.cchProtocol; ++i)
			{
				char lChar = static_cast<char>(tolower(pURL.pszProtocol[i]));
				this->url_.protocol.append(&lChar, 1);
			}
		}
		else
		{
			this->url_.protocol.append("http");
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

		auto pos = server.find('/');
		if (pos == std::string::npos)
		{
			this->url_.server = server;
			this->url_.document = "/";
		}
		else
		{
			this->url_.server = server.substr(0, pos);
			this->url_.document = server.substr(pos);
		}

		this->url_.port.clear();

		pos = this->url_.server.find(':');
		if (pos != std::string::npos)
		{
			this->url_.port = this->url_.server.substr(pos + 1);
			this->url_.server = this->url_.server.substr(0, pos);
		}

		this->url_.raw.clear();
		this->url_.raw.append(this->url_.protocol);
		this->url_.raw.append("://");
		this->url_.raw.append(this->url_.server);

		if (!this->url_.port.empty())
		{
			this->url_.raw.append(":");
			this->url_.raw.append(this->url_.port);
		}

		this->url_.raw.append(this->url_.document);
		this->isFTP_ = (this->url_.protocol == "ftp");
	}

	std::string WebIO::buildPostBody(const params& params)
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

	std::string WebIO::postFile(const std::string& url, const std::string& data, const std::string& fieldName, const std::string& fileName)
	{
		this->setURL(url);
		return this->postFile(data, fieldName, fileName);
	}

	std::string WebIO::postFile(const std::string& data, std::string fieldName, std::string fileName)
	{
		params headers;

		std::string boundary = "----WebKitFormBoundaryHoLVocRsBxs71fU6";
		headers["Content-Type"] = "multipart/form-data, boundary=" + boundary;

		String::Replace(fieldName, "\"", "\\\"");
		String::Replace(fieldName, "\\", "\\\\");
		String::Replace(fileName, "\"", "\\\"");
		String::Replace(fileName, "\\", "\\\\");

		std::string body = "--" + boundary + "\r\n";
		body += "Content-Disposition: form-data; name=\"";
		body += fieldName;
		body += "\"; filename=\"";
		body += fileName;
		body += "\"\r\n";
		body += "Content-Type: application/octet-stream\r\n\r\n";
		body += data + "\r\n";
		body += "--" + boundary + "--\r\n";

		headers["Content-Length"] = String::VA("%u", body.size());

		return this->execute("POST", body, headers);
	}

	std::string WebIO::post(const std::string& url, const std::string& body, bool* success)
	{
		this->setURL(url);
		return this->post(body, success);
	}

	std::string WebIO::post(const std::string& url, const params& params, bool* success)
	{
		this->setURL(url);
		return this->post(params, success);
	}

	std::string WebIO::post(const params& params, bool* success)
	{
		return this->post(this->buildPostBody(params), success);
	}

	std::string WebIO::post(const std::string& body, bool* success)
	{
		const params params;
		return this->execute("POST", body, params, success);
	}

	std::string WebIO::get(const std::string& url, bool* success)
	{
		this->setURL(url);
		return this->get(success);
	}

	std::string WebIO::get(bool* success)
	{
		const params params;
		return this->execute("GET", "", params, success);
	}

	bool WebIO::openConnection()
	{
		WORD wPort = INTERNET_DEFAULT_HTTP_PORT;
		DWORD dwService = INTERNET_SERVICE_HTTP;
		DWORD dwFlag = 0;

		if (this->isFTP_)
		{
			wPort = INTERNET_DEFAULT_FTP_PORT;
			dwService = INTERNET_SERVICE_FTP;
			dwFlag = INTERNET_FLAG_PASSIVE;
		}
		else if (this->isSecuredConnection())
		{
			wPort = INTERNET_DEFAULT_HTTPS_PORT;
		}

		if (!this->url_.port.empty())
		{
			wPort = static_cast<WORD>(atoi(this->url_.port.data()));
		}

		this->hConnect_ = InternetConnectA(this->hSession_, this->url_.server.data(), wPort, this->username_.data(), this->password_.data(), dwService, dwFlag, NULL);

		return (this->hConnect_ && this->hConnect_ != INVALID_HANDLE_VALUE);
	}

	void WebIO::closeConnection()
	{
		if (this->hFile_ && this->hFile_ != INVALID_HANDLE_VALUE) InternetCloseHandle(this->hFile_);
		if (this->hConnect_ && this->hConnect_ != INVALID_HANDLE_VALUE) InternetCloseHandle(this->hConnect_);
	}

	WebIO* WebIO::setTimeout(DWORD msec)
	{
		this->timeout_ = msec;
		return this;
	}

	std::string WebIO::execute(const char* command, const std::string& body, const params& headers, bool* success)
	{
		if (success) *success = false;
		if (!this->openConnection()) return {};

		static const char* acceptTypes[] = { "application/x-www-form-urlencoded", nullptr };

		DWORD dwFlag = INTERNET_FLAG_RELOAD | (this->isSecuredConnection() ? INTERNET_FLAG_SECURE : 0);

		InternetSetOptionA(this->hConnect_, INTERNET_OPTION_CONNECT_TIMEOUT, &this->timeout_, sizeof(this->timeout_));
		InternetSetOptionA(this->hConnect_, INTERNET_OPTION_RECEIVE_TIMEOUT, &this->timeout_, sizeof(this->timeout_));
		InternetSetOptionA(this->hConnect_, INTERNET_OPTION_SEND_TIMEOUT, &this->timeout_, sizeof(this->timeout_));

		this->hFile_ = HttpOpenRequestA(this->hConnect_, command, this->url_.document.data(), nullptr, nullptr, acceptTypes, dwFlag, NULL);

		if (!this->hFile_ || this->hFile_ == INVALID_HANDLE_VALUE)
		{
			this->closeConnection();
			return {};
		}

		params params = headers;
		if (!params.contains("Content-Type"))
		{
			params["Content-Type"] = "application/x-www-form-urlencoded";
		}

		std::string finalHeaders;

		for (auto i = params.begin(); i != params.end(); ++i)
		{
			finalHeaders.append(i->first);
			finalHeaders.append(": ");
			finalHeaders.append(i->second);
			finalHeaders.append("\r\n");
		}

		if (HttpSendRequestA(this->hFile_, finalHeaders.data(), finalHeaders.size(), const_cast<char*>(body.data()), body.size() + 1) == FALSE)
		{
			return {};
		}

		DWORD statusCode = 404;
		DWORD length = sizeof(statusCode);
		if (HttpQueryInfoA(this->hFile_, HTTP_QUERY_FLAG_NUMBER | HTTP_QUERY_STATUS_CODE, &statusCode, &length, nullptr) == FALSE || (statusCode != 200 && statusCode != 201))
		{
			this->closeConnection();
			return {};
		}

		DWORD contentLength = 0;
		length = sizeof(statusCode);
		if (HttpQueryInfoA(this->hFile_, HTTP_QUERY_FLAG_NUMBER | HTTP_QUERY_CONTENT_LENGTH, &contentLength, &length, nullptr) == FALSE)
		{
			contentLength = 0;
		}

		std::string returnBuffer;
		returnBuffer.reserve(contentLength);

		DWORD size{};
		char buffer[0x2001]{};

		while (InternetReadFile(this->hFile_, buffer, sizeof(buffer) - 1, &size))
		{
			if (this->cancel_)
			{
				this->closeConnection();
				return {};
			}

			returnBuffer.append(buffer, size);
			if (this->progressCallback) this->progressCallback(returnBuffer.size(), contentLength);
			if (!size) break;
		}

		this->closeConnection();

		if (success) *success = true;
		return returnBuffer;
	}

	bool WebIO::isSecuredConnection() const
	{
		return this->url_.protocol == "https"s;
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
		return (FtpSetCurrentDirectoryA(this->hConnect_, directory.data()) == TRUE);
	}

	bool WebIO::setRelativeDirectory(std::string directory)
	{
		std::string currentDir;

		if (this->getDirectory(&currentDir))
		{
			FormatPath(directory, true);
			FormatPath(currentDir, true);

			char path[MAX_PATH]{};
			PathCombineA(path, currentDir.data(), directory.data());

			std::string newPath(path);
			FormatPath(newPath, false);

			return this->setDirectory(newPath);
		}

		return false;
	}

	bool WebIO::getDirectory(std::string* directory) const
	{
		directory->clear();

		char currentDir[MAX_PATH]{};
		DWORD size = sizeof(currentDir);

		if (FtpGetCurrentDirectoryA(this->hConnect_, currentDir, &size) == TRUE)
		{
			directory->append(currentDir, size);
			return true;
		}

		return false;
	}

	void WebIO::FormatPath(std::string& path, bool win)
	{
		std::size_t nPos;
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

	std::string WebIO::GetCacheBuster()
	{
		return "?" + std::to_string(
			std::chrono::duration_cast<std::chrono::nanoseconds>(
				std::chrono::system_clock::now().time_since_epoch()).count());
	}

	bool WebIO::createDirectory(const std::string& directory) const
	{
		return (FtpCreateDirectoryA(this->hConnect_, directory.data()) == TRUE);
	}

	// Recursively delete a directory
	bool WebIO::deleteDirectory(const std::string& directory)
	{
		std::string tempDir;
		this->getDirectory(&tempDir);

		this->setRelativeDirectory(directory);

		std::vector<std::string> list;

		this->listFiles(".", list);
		for (const auto& file : list)
		{
			this->deleteFile(file);
		}

		this->listDirectories(".", list);
		for (auto& dir : list) this->deleteDirectory(dir);

		this->setDirectory(tempDir);

		return (FtpRemoveDirectoryA(this->hConnect_, directory.data()) == TRUE);
	}

	bool WebIO::renameDirectory(const std::string& directory, const std::string& newDir) const
	{
		return (FtpRenameFileA(this->hConnect_, directory.data(), newDir.data()) == TRUE); // According to the internet, this should work
	}

	bool WebIO::listElements(const std::string& directory, std::vector<std::string>& list, bool files)
	{
		list.clear();

		WIN32_FIND_DATAA findFileData;
		bool result = false;
		const DWORD dwAttribute = (files ? FILE_ATTRIBUTE_NORMAL : FILE_ATTRIBUTE_DIRECTORY);

		// Any filename.
		std::string tempDir;
		this->getDirectory(&tempDir);
		this->setRelativeDirectory(directory);

		this->hFile_ = FtpFindFirstFileA(this->hConnect_, "*", &findFileData, INTERNET_FLAG_RELOAD, NULL);

		if (this->hFile_ && this->hFile_ != INVALID_HANDLE_VALUE)
		{
			do
			{
				//if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_OFFLINE) continue;
				//if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) continue;

				if (findFileData.dwFileAttributes == dwAttribute) // No bitwise flag check, as it might return archives/offline/hidden or other files/dirs
				{
					list.emplace_back(findFileData.cFileName);
					result = true;
				}
			} while (InternetFindNextFileA(this->hFile_, &findFileData));

			InternetCloseHandle(this->hFile_);
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

	bool WebIO::deleteFile(const std::string& file) const
	{
		return (FtpDeleteFileA(this->hConnect_, file.data()) == TRUE);
	}

	bool WebIO::renameFile(const std::string& file, const std::string& newFile) const
	{
		return (FtpRenameFileA(this->hConnect_, file.data(), newFile.data()) == TRUE);
	}

	bool WebIO::downloadFile(const std::string& file, const std::string& localFile) const
	{
		return (FtpGetFileA(this->hConnect_, file.data(), localFile.data(), FALSE, NULL, FTP_TRANSFER_TYPE_BINARY, NULL) == TRUE);
	}

	bool WebIO::uploadFile(const std::string& file, const std::string& localFile) const
	{
		return (FtpPutFileA(this->hConnect_, localFile.data(), file.data(), FTP_TRANSFER_TYPE_BINARY, NULL) == TRUE);
	}

	bool WebIO::uploadFileData(const std::string& file, const std::string& data)
	{
		bool result = false;
		this->hFile_ = FtpOpenFileA(this->hConnect_, file.data(), GENERIC_WRITE, INTERNET_FLAG_TRANSFER_BINARY | INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_RELOAD, NULL);

		if (this->hFile_)
		{
			DWORD size = 0;
			if (InternetWriteFile(this->hFile_, data.data(), data.size(), &size) == TRUE)
			{
				result = (size == data.size());
			}

			InternetCloseHandle(this->hFile_);
		}

		return result;
	}

	bool WebIO::downloadFileData(const std::string& file, std::string& data)
	{
		data.clear();

		this->hFile_ = FtpOpenFileA(this->hConnect_, file.data(), GENERIC_READ, INTERNET_FLAG_TRANSFER_BINARY | INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_RELOAD, NULL);

		if (this->hFile_)
		{
			DWORD size = 0;
			char buffer[0x2001]{};

			while (InternetReadFile(this->hFile_, buffer, sizeof(buffer) - 1, &size))
			{
				data.append(buffer, size);
				if (!size) break;
			}

			InternetCloseHandle(this->hFile_);
			return true;
		}

		return false;
	}

	void WebIO::setProgressCallback(const Slot<void(std::size_t, std::size_t)>& callback)
	{
		this->progressCallback = callback;
	}

	void WebIO::cancelDownload()
	{
		this->cancel_ = true;
	}
}
