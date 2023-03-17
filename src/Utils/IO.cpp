#include <STDInclude.hpp>

namespace Utils::IO
{
	bool FileExists(const std::string& file)
	{
		//return std::ifstream(file).good();
		return GetFileAttributesA(file.data()) != INVALID_FILE_ATTRIBUTES;
	}

	bool WriteFile(const std::string& file, const std::string& data, bool append)
	{
		const auto pos = file.find_last_of("/\\");
		if (pos != std::string::npos)
		{
			CreateDir(file.substr(0, pos));
		}

		std::ofstream stream(file, std::ios::binary | std::ofstream::out | (append ? std::ofstream::app : std::ofstream::out));

		if (stream.is_open())
		{
			stream.write(data.data(), static_cast<std::streamsize>(data.size()));
			stream.close();
			return true;
		}

		return false;
	}

	std::string ReadFile(const std::string& file)
	{
		std::string data;
		ReadFile(file, &data);
		return data;
	}

	bool ReadFile(const std::string& file, std::string* data)
	{
		if (!data) return false;
		data->clear();

		if (FileExists(file))
		{
			std::ifstream stream(file, std::ios::binary);
			if (!stream.is_open()) return false;

			stream.seekg(0, std::ios::end);
			const std::streamsize size = stream.tellg();
			stream.seekg(0, std::ios::beg);

			if (size > -1)
			{
				data->resize(static_cast<std::string::size_type>(size));
				stream.read(data->data(), size);
				stream.close();
				return true;
			}
		}

		return false;
	}

	bool RemoveFile(const std::string& file)
	{
		return DeleteFileA(file.data()) == TRUE;
	}

	std::size_t FileSize(const std::string& file)
	{
		if (FileExists(file))
		{
			std::ifstream stream(file, std::ios::binary);

			if (stream.good())
			{
				stream.seekg(0, std::ios::end);
				return static_cast<std::size_t>(stream.tellg());
			}
		}

		return 0;
	}

	bool CreateDir(const std::string& dir)
	{
		return std::filesystem::create_directories(dir);
	}

	bool DirectoryExists(const std::filesystem::path& directory)
	{
		return std::filesystem::is_directory(directory);
	}

	bool DirectoryIsEmpty(const std::filesystem::path& directory)
	{
		return std::filesystem::is_empty(directory);
	}

	std::vector<std::filesystem::directory_entry> ListFiles(const std::filesystem::path& directory, const bool recursive)
	{
		std::vector<std::filesystem::directory_entry> files;

		if (recursive)
		{
			for (auto& file : std::filesystem::recursive_directory_iterator(directory))
			{
				files.push_back(file);
			}
		}
		else
		{
			for (auto& file : std::filesystem::directory_iterator(directory))
			{
				files.push_back(file);
			}
		}

		return files;
	}
}
