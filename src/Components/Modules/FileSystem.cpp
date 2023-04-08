#include <STDInclude.hpp>

namespace Components
{
	std::mutex FileSystem::Mutex;
	std::recursive_mutex FileSystem::FSMutex;
	Utils::Memory::Allocator FileSystem::MemAllocator;

	void FileSystem::File::read(Game::FsThread thread)
	{
		std::lock_guard _(FSMutex);

		assert(!filePath.empty());

		int handle;
		const auto len = Game::FS_FOpenFileReadForThread(filePath.data(), &handle, thread);

		if (!handle)
		{
			return;
		}

		auto* buf = AllocateFile(len + 1);

		[[maybe_unused]] auto bytesRead = Game::FS_Read(buf, len, handle);

		assert(bytesRead == len);

		buf[len] = '\0';

		Game::FS_FCloseFile(handle);

		this->buffer.append(buf, len);
		FreeFile(buf);
	}

	void FileSystem::RawFile::read()
	{
		this->buffer.clear();

		auto* rawfile = Game::DB_FindXAssetHeader(Game::XAssetType::ASSET_TYPE_RAWFILE, this->filePath.data()).rawfile;
		if (!rawfile || Game::DB_IsXAssetDefault(Game::XAssetType::ASSET_TYPE_RAWFILE, this->filePath.data())) return;

		this->buffer.resize(Game::DB_GetRawFileLen(rawfile));
		Game::DB_GetRawBuffer(rawfile, this->buffer.data(), static_cast<int>(this->buffer.size()));
	}

	FileSystem::FileReader::FileReader(std::string file) : handle(0), name(std::move(file))
	{
		this->size = Game::FS_FOpenFileReadCurrentThread(this->name.data(), &this->handle);
	}

	FileSystem::FileReader::~FileReader()
	{
		if (this->exists() && this->handle)
		{
			Game::FS_FCloseFile(this->handle);
		}
	}

	bool FileSystem::FileReader::exists() const noexcept
	{
		return (this->size >= 0 && this->handle);
	}

	std::string FileSystem::FileReader::getName() const
	{
		return this->name;
	}

	int FileSystem::FileReader::getSize() const noexcept
	{
		return this->size;
	}

	std::string FileSystem::FileReader::getBuffer() const
	{
		Utils::Memory::Allocator allocator;
		if (!this->exists()) return {};

		const auto position = Game::FS_FTell(this->handle);
		this->seek(0, Game::FS_SEEK_SET);

		char* buffer = allocator.allocateArray<char>(this->size);
		if (!this->read(buffer, this->size))
		{
			this->seek(position, Game::FS_SEEK_SET);
			return {};
		}

		this->seek(position, Game::FS_SEEK_SET);

		return {buffer, static_cast<std::size_t>(this->size)};
	}

	bool FileSystem::FileReader::read(void* buffer, std::size_t _size) const noexcept
	{
		if (!this->exists() || static_cast<std::size_t>(this->size) < _size || Game::FS_Read(buffer, static_cast<int>(_size), this->handle) != static_cast<int>(_size))
		{
			return false;
		}

		return true;
	}

	void FileSystem::FileReader::seek(int offset, int origin) const
	{
		if (this->exists())
		{
			Game::FS_Seek(this->handle, offset, origin);
		}
	}

	void FileSystem::FileWriter::write(const std::string& data) const
	{
		if (this->handle)
		{
			Game::FS_Write(data.data(), static_cast<int>(data.size()), this->handle);
		}
	}

	void FileSystem::FileWriter::open(bool append)
	{
		if (append)
		{
			this->handle = Game::FS_FOpenFileAppend(this->filePath.data());
		}
		else
		{
			this->handle = Game::FS_FOpenFileWrite(this->filePath.data());
		}
	}

	void FileSystem::FileWriter::close()
	{
		if (this->handle)
		{
			Game::FS_FCloseFile(this->handle);
			this->handle = 0;
		}
	}

	std::filesystem::path FileSystem::GetAppdataPath()
	{
		PWSTR path;
		if (!SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &path)))
		{
			throw std::runtime_error("Failed to read APPDATA path!");
		}

		auto _0 = gsl::finally([&path]
		{
			CoTaskMemFree(path);
		});

		return std::filesystem::path(path) / "xlabs";
	}

	std::vector<std::string> FileSystem::GetFileList(const std::string& path, const std::string& extension)
	{
		std::vector<std::string> fileList;

		auto numFiles = 0;
		const auto** files = Game::FS_ListFiles(path.data(), extension.data(), Game::FS_LIST_PURE_ONLY, &numFiles, 10);

		if (files)
		{
			for (int i = 0; i < numFiles; ++i)
			{
				if (files[i])
				{
					fileList.emplace_back(files[i]);
				}
			}

			Game::FS_FreeFileList(files, 10);
		}

		return fileList;
	}

	std::vector<std::string> FileSystem::GetSysFileList(const std::string& path, const std::string& extension, bool folders)
	{
		std::vector<std::string> fileList;

		auto numFiles = 0;
		const auto** files = Game::Sys_ListFiles(path.data(), extension.data(), nullptr, &numFiles, folders);

		if (files)
		{
			for (int i = 0; i < numFiles; ++i)
			{
				if (files[i])
				{
					fileList.emplace_back(files[i]);
				}
			}

			Game::Sys_FreeFileList(files);
		}

		return fileList;
	}

	bool FileSystem::_DeleteFile(const std::string& folder, const std::string& file)
	{
		char path[MAX_PATH]{};
		Game::FS_BuildPathToFile((*Game::fs_basepath)->current.string, reinterpret_cast<char*>(0x63D0BB8), Utils::String::VA("%s/%s", folder.data(), file.data()), reinterpret_cast<char**>(&path));
		return Game::FS_Remove(path);
	}

	int FileSystem::ReadFile(const char* path, char** buffer)
	{
		if (!buffer) return -1;
		if (!path) return -1;

		std::lock_guard _(Mutex);
		FileReader reader(path);

		int size = reader.getSize();
		if (reader.exists() && size >= 0)
		{
			*buffer = AllocateFile(size + 1);
			if (reader.read(*buffer, size)) return size;

			FreeFile(*buffer);
			*buffer = nullptr;
		}

		return -1;
	}

	char* FileSystem::AllocateFile(int size)
	{
		return MemAllocator.allocateArray<char>(size);
	}

	void FileSystem::FreeFile(void* buffer)
	{
		MemAllocator.free(buffer);
	}

	void FileSystem::RegisterFolder(const char* folder)
	{
		const std::string fs_cdpath = (*Game::fs_cdpath)->current.string;
		const std::string fs_basepath = (*Game::fs_basepath)->current.string;
		const std::string fs_homepath = (*Game::fs_homepath)->current.string;

		if (!fs_cdpath.empty())   Game::FS_AddLocalizedGameDirectory(fs_cdpath.data(),   folder);
		if (!fs_basepath.empty()) Game::FS_AddLocalizedGameDirectory(fs_basepath.data(), folder);
		if (!fs_homepath.empty()) Game::FS_AddLocalizedGameDirectory(fs_homepath.data(), folder);
	}

	void FileSystem::RegisterFolders()
	{
		if (ZoneBuilder::IsEnabled())
		{
			RegisterFolder("zonedata");
		}

		RegisterFolder("userraw");
	}

	__declspec(naked) void FileSystem::StartupStub()
	{
		__asm
		{
			pushad
			push esi
			call FileSystem::RegisterFolders
			pop esi
			popad

			mov edx, ds:63D0CC0h

			push 48264Dh
			retn
		}
	}

	int FileSystem::Cmd_Exec_f_Stub(const char* s0, [[maybe_unused]] const char* s1)
	{
		int f;
		auto len = Game::FS_FOpenFileByMode(s0, &f, Game::FS_READ);
		if (len < 0)
		{
			return 1; // Not found
		}

		Game::FS_FCloseFile(f);
		return 0; // Found
	}

	void FileSystem::FsStartupSync(const char* a1)
	{
		std::lock_guard _(FSMutex);
		return Utils::Hook::Call<void(const char*)>(0x4823A0)(a1); // FS_Startup
	}

	void FileSystem::FsRestartSync(int localClientNum, int checksumFeed)
	{
		std::lock_guard _(FSMutex);
		Maps::GetUserMap()->freeIwd();
		Utils::Hook::Call<void(int, int)>(0x461A50)(localClientNum, checksumFeed); // FS_Restart
		Maps::GetUserMap()->reloadIwd();
	}

	void FileSystem::FsShutdownSync(int closemfp)
	{
		std::lock_guard _(FSMutex);
		Maps::GetUserMap()->freeIwd();
		Utils::Hook::Call<void(int)>(0x4A46C0)(closemfp); // FS_Shutdown
	}

	void FileSystem::DelayLoadImagesSync()
	{
		std::lock_guard _(FSMutex);
		return Utils::Hook::Call<void()>(0x494060)(); // DB_LoadDelayedImages
	}

	int FileSystem::LoadTextureSync(Game::GfxImageLoadDef** loadDef, Game::GfxImage* image)
	{
		std::lock_guard _(FSMutex);
		return Game::Load_Texture(loadDef, image);
	}

	void FileSystem::IwdFreeStub(Game::iwd_t* iwd)
	{
		Maps::GetUserMap()->handlePackfile(iwd);
		Utils::Hook::Call<void(void*)>(0x4291A0)(iwd);
	}

	const char* FileSystem::Sys_DefaultInstallPath_Hk()
	{
		static auto current_path = std::filesystem::current_path().string();
		return current_path.data();
	}

	FileSystem::FileSystem()
	{
		// Thread safe file system interaction
		Utils::Hook(0x4F4BFF, AllocateFile, HOOK_CALL).install()->quick();
		Utils::Hook(Game::FS_FreeFile, FreeFile, HOOK_JUMP).install()->quick();

		// Filesystem config checks
		Utils::Hook(0x6098FD, Cmd_Exec_f_Stub, HOOK_CALL).install()->quick();

		// Don't strip the folders from the config name (otherwise our ExecIsFSStub fails)
		Utils::Hook::Nop(0x6098F2, 5);

		// Register additional folders
		Utils::Hook(0x482647, StartupStub, HOOK_JUMP).install()->quick();

		// exec whitelist removal
		Utils::Hook::Nop(0x609685, 5);
		Utils::Hook::Nop(0x60968C, 2);

		// ignore 'no iwd files found in main'
		Utils::Hook::Nop(0x642A4B, 5);

		// Ignore bad magic, when trying to free hunk when it's already cleared
		Utils::Hook::Set<std::uint16_t>(0x49AACE, 0xC35E);

		// Synchronize filesystem starts
		Utils::Hook(0x4290C6, FsStartupSync, HOOK_CALL).install()->quick(); // FS_InitFilesystem
		Utils::Hook(0x461A88, FsStartupSync, HOOK_CALL).install()->quick(); // FS_Restart

		// Synchronize filesystem restarts
		Utils::Hook(0x4A745B, FsRestartSync, HOOK_CALL).install()->quick(); // SV_SpawnServer
		Utils::Hook(0x4C8609, FsRestartSync, HOOK_CALL).install()->quick(); // FS_ConditionalRestart
		Utils::Hook(0x5AC68E, FsRestartSync, HOOK_CALL).install()->quick(); // CL_ParseServerMessage

		// Synchronize filesystem stops
		Utils::Hook(0x461A55, FsShutdownSync, HOOK_CALL).install()->quick(); // FS_Restart
		Utils::Hook(0x4D40DB, FsShutdownSync, HOOK_CALL).install()->quick(); // Com_Quitf

		// Synchronize db image loading
		Utils::Hook(0x415AB8, DelayLoadImagesSync, HOOK_CALL).install()->quick();
		Utils::Hook(0x4D32BC, LoadTextureSync, HOOK_CALL).install()->quick();

		// Handle IWD freeing
		Utils::Hook(0x642F60, IwdFreeStub, HOOK_CALL).install()->quick();

		// Set the working dir based on info from the Xlabs launcher
		Utils::Hook(0x4326E0, Sys_DefaultInstallPath_Hk, HOOK_JUMP).install()->quick();
	}

	FileSystem::~FileSystem()
	{
		assert(FileSystem::MemAllocator.empty());
	}
}
