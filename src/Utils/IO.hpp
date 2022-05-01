#pragma once

namespace Utils
{
	namespace IO
	{
		bool FileExists(const std::string& file);
		bool WriteFile(const std::string& file, const std::string& data, bool append = false);
		bool ReadFile(const std::string& file, std::string* data);
		std::string ReadFile(const std::string& file);
		bool RemoveFile(const std::string& file);
		size_t FileSize(const std::string& file);
		bool CreateDir(const std::string& dir);
		bool DirectoryExists(const std::string& file);
		bool DirectoryIsEmpty(const std::string& file);
		std::vector<std::string> ListFiles(const std::string& dir);
	}
}
