#pragma once

namespace Utils
{
	namespace IO
	{
		bool FileExists(std::string file);
		bool WriteFile(std::string file, std::string data, bool append = false);
		bool ReadFile(std::string file, std::string* data);
		std::string ReadFile(std::string file);
		size_t FileSize(std::string file);
		bool CreateDir(std::string dir);
		bool DirectoryExists(std::string file);
		bool DirectoryIsEmpty(std::string file);
		std::vector<std::string> ListFiles(std::string dir);
	}
}
