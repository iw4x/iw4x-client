#pragma once

namespace Utils::IO
{
	[[nodiscard]] bool FileExists(const std::string& file);
	bool WriteFile(const std::string& file, const std::string& data, bool append = false);
	bool ReadFile(const std::string& file, std::string* data);
	[[nodiscard]] std::string ReadFile(const std::string& file);
	bool RemoveFile(const std::string& file);
	[[nodiscard]] std::size_t FileSize(const std::string& file);
	bool CreateDir(const std::string& dir);
	[[nodiscard]] bool DirectoryExists(const std::filesystem::path& directory);
	[[nodiscard]] bool DirectoryIsEmpty(const std::filesystem::path& directory);
	[[nodiscard]] std::vector<std::filesystem::directory_entry> ListFiles(const std::filesystem::path& directory, bool recursive = false);
}
