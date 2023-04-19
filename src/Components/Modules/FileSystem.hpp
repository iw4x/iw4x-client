#pragma once

namespace Components
{
	class FileSystem : public Component
	{
	public:
		class AbstractFile
		{
		public:
			virtual ~AbstractFile() = default;

			[[nodiscard]] virtual bool exists() const noexcept = 0;
			[[nodiscard]] virtual std::string getName() const = 0;
			[[nodiscard]] virtual std::string& getBuffer() = 0;

			virtual explicit operator bool()
			{
				return this->exists();
			}
		};

		class File : public AbstractFile
		{
		public:
			File() = default;
			File(std::string file) : filePath{std::move(file)} { this->read(); }
			File(std::string file, Game::FsThread thread) : filePath{std::move(file)} { this->read(thread); }

			[[nodiscard]] bool exists() const noexcept override { return !this->buffer.empty(); }
			[[nodiscard]] std::string getName() const override { return this->filePath; }
			[[nodiscard]] std::string& getBuffer() override { return this->buffer; }

		private:
			std::string filePath;
			std::string buffer;

			void read(Game::FsThread thread = Game::FS_THREAD_MAIN);
		};

		class RawFile : public AbstractFile
		{
		public:
			RawFile() = default;
			RawFile(std::string file) : filePath(std::move(file)) { this->read(); }

			[[nodiscard]] bool exists() const noexcept override { return !this->buffer.empty(); }
			[[nodiscard]] std::string getName() const override { return this->filePath; }
			[[nodiscard]] std::string& getBuffer() override { return this->buffer; }

		private:
			std::string filePath;
			std::string buffer;

			void read();
		};

		class FileReader
		{
		public:
			FileReader() : handle(0), size(-1) {}
			FileReader(std::string file);
			~FileReader();

			[[nodiscard]] bool exists() const noexcept;
			[[nodiscard]] std::string getName() const;
			[[nodiscard]] std::string getBuffer() const;
			[[nodiscard]] int getSize() const noexcept;
			bool read(void* buffer, std::size_t size) const noexcept;
			void seek(int offset, int origin) const;

		private:
			int handle;
			int size;
			std::string name;
		};

		class FileWriter
		{
		public:
			FileWriter(std::string file, bool append = false) : handle(0), filePath(std::move(file)) { this->open(append); }
			~FileWriter() { this->close(); }

			void write(const std::string& data) const;

		private:
			int handle;
			std::string filePath;

			void open(bool append = false);
			void close();
		};

		FileSystem();
		~FileSystem();

		static std::filesystem::path GetAppdataPath();
		static std::vector<std::string> GetFileList(const std::string& path, const std::string& extension);
		static std::vector<std::string> GetSysFileList(const std::string& path, const std::string& extension, bool folders = false);
		static bool _DeleteFile(const std::string& folder, const std::string& file);

	private:
		static std::mutex Mutex;
		static std::recursive_mutex FSMutex;
		static Utils::Memory::Allocator MemAllocator;

		static int ReadFile(const char* path, char** buffer);
		static char* AllocateFile(int size);
		static void FreeFile(void* buffer);

		static void RegisterFolder(const char* folder);

		static void RegisterFolders();
		static void StartupStub();
		static int Cmd_Exec_f_Stub(const char* s0, const char* s1);

		static void FsStartupSync(const char* a1);
		static void FsRestartSync(int localClientNum, int checksumFeed);
		static void FsShutdownSync(int closemfp);
		static void DelayLoadImagesSync();
		static int LoadTextureSync(Game::GfxImageLoadDef** loadDef, Game::GfxImage* image);

		static void IwdFreeStub(Game::iwd_t* iwd);

		static const char* Sys_DefaultInstallPath_Hk();
	};
}
