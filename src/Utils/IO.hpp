namespace Utils
{
	namespace IO
	{
		bool FileExists(std::string file);
		void WriteFile(std::string file, std::string data, bool append = false);
		std::string ReadFile(std::string file);
		bool CreateDirectory(std::string dir);
		std::vector<std::string> ListFiles(std::string dir);
	}
}
