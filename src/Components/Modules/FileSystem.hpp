namespace Components
{
	class FileSystem : public Component
	{
	public:

		class File
		{
		public:
			File() : Size(-1), Name(), Handle(0) {};
			File(std::string file);
			~File();

			bool Exists();
			std::string GetName();
			std::string GetBuffer();
			bool Read(void* buffer, size_t size);
			void Seek(int offset, int origin);

		private:
			int Handle;
			int Size;
			std::string Name;
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

#ifdef DEBUG
		const char* GetName() { return "FileSystem"; };
#endif

		static std::vector<std::string> GetFileList(std::string path, std::string extension);
		static std::vector<std::string> GetSysFileList(std::string path, std::string extension, bool folders = false);
		static void DeleteFile(std::string folder, std::string file);

	private:

		static void RegisterFolder(const char* folder);

		static void RegisterFolders();
		static void StartupStub();
		static int ExecIsFSStub(const char* execFilename);
	};
}
