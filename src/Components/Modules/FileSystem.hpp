namespace Components
{
	class FileSystem : public Component
	{
	public:

		class File
		{
		public:
			//File() {};
			File(std::string file) : FilePath(file) { this->Read(); };

			bool Exists() { return this->Buffer.size() > 0; };
			std::string GetName() { return this->FilePath; };
			std::string& GetBuffer() { return this->Buffer; };

		private:
			std::string FilePath;
			std::string Buffer;

			void Read();
		};

		class FileWriter
		{
		public:
			FileWriter(std::string file) : FilePath(file), Handle(0) { this->Open(); };
			~FileWriter() { this->Close(); };

			void Write(std::string data);

		private:
			int Handle;
			std::string FilePath;

			void Open();
			void Close();
		};

		FileSystem();
		const char* GetName() { return "FileSystem"; };

		static std::vector<std::string> GetFileList(std::string path, std::string extension);
		static void DeleteFile(std::string folder, std::string file);

	private:
		static int ExecIsFSStub(const char* execFilename);
	};
}
