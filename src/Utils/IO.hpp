#pragma once

namespace Utils
{
	namespace IO
	{
		bool FileExists(std::string file);
		bool WriteFile(std::string file, std::string data, bool append = false);
		bool ReadFile(std::string file, std::string* data);
		size_t FileSize(std::string file);
		std::string ReadFile(std::string file);
		bool CreateDirectory(std::string dir);
		std::vector<std::string> ListFiles(std::string dir);
	}
}
