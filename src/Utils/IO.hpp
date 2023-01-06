#pragma once

namespace Utils::IO
{
	bool FileExists(const std::string& file);
	bool WriteFile(const std::string& file, const std::string& data, bool append = false);
	bool ReadFile(const std::string& file, std::string* data);
	std::string ReadFile(const std::string& file);
	bool RemoveFile(const std::string& file);
	std::size_t FileSize(const std::string& file);
	bool CreateDir(const std::string& dir);
	bool DirectoryExists(const std::filesystem::path& directory);
	bool DirectoryIsEmpty(const std::filesystem::path& directory);
	std::vector<std::string> ListFiles(const std::filesystem::path& directory, bool recursive = false);
}
