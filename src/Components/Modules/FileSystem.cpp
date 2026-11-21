#include "FileSystem.hpp"
#include "Maps.hpp"

namespace Components
{
  // patch max file amount returned by Sys_ListFiles
  constexpr auto FILE_COUNT_MULTIPLIER = 8;
  constexpr auto NEW_MAX_FILES_LISTED = 8191 * FILE_COUNT_MULTIPLIER;

  std::mutex FileSystem::Mutex;
  std::recursive_mutex FileSystem::FSMutex;
  Utils::Memory::Allocator FileSystem::MemAllocator;

  int (*FileSystem::FS_FOpenFileReadForThread_Original)(const char* filename, Game::fileHandle_t* file, int thread);

  void FileSystem::File::read(Game::FsThread thread)
  {
    std::lock_guard _(FSMutex);

    assert(!filePath.empty());

    Game::fileHandle_t handle;
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
    // Note that x64 inlines FS_FOpenFileAppend(), which we call on x86, into
    // FS_FOpenFileByMode(), so here we open the file through the latter. It
    // sets the handle to 0 if the file can't be opened, the same as
    // FS_FOpenFileWrite().
    //
    if (append)
    {
      Game::FS_FOpenFileByMode(this->filePath.data(), &this->handle, Game::FS_APPEND);
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

    return std::filesystem::path(path) / "iw4x";
  }

  std::vector<std::string> FileSystem::GetFileList(const std::string& path, const std::string& extension,  Game::FsListBehavior_e behaviour)
  {
    std::vector<std::string> fileList;

    auto numFiles = 0;
    const auto** files = Game::FS_ListFiles(path.data(), extension.data(), behaviour, &numFiles, 10);

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

  // Note that x64 inlines FS_BuildPathToFile(), which we call on x86 and
  // which only passes its arguments on to FS_BuildOSPath(), so here we call
  // the latter.
  //
  bool FileSystem::_DeleteFile(const std::string& folder, const std::string& file)
  {
    char path[MAX_PATH]{};
    Game::FS_BuildOSPath((*Game::fs_basepath)->current.string, Game::fs_gamedir, Utils::String::VA("%s/%s", folder.data(), file.data()), path);
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

  // Note that on x86 we also register the directory of the zone builder's
  // sources, and the zone builder isn't part of the x64 build.
  //
  void FileSystem::RegisterFolders()
  {
    RegisterFolder("userraw");
  }

  int FileSystem::Cmd_Exec_f_Stub(const char* s0, [[maybe_unused]] const char* s1)
  {
    Game::fileHandle_t f;
    const auto len = Game::FS_FOpenFileByMode(s0, &f, Game::FS_READ);
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
    return Utils::Hook::Call<void(const char*)>(0x140276D70)(a1); // FS_Startup
  }

  void FileSystem::FsRestartSync(int localClientNum, int checksumFeed)
  {
    std::lock_guard _(FSMutex);
    Maps::GetUserMap()->freeIwd();
    Utils::Hook::Call<void(int, int)>(0x1402764D0)(localClientNum, checksumFeed); // FS_Restart
    Maps::GetUserMap()->reloadIwd();
  }

  // Note that x64 compiles out FS_ConditionalRestart() and so we reimplement
  // it from x86, through FsRestartSync() whose call we hook in it on x86.
  //
  bool FileSystem::FS_ConditionalRestart(const int localClientNum, const int checksumFeed)
  {
    if (!Utils::Hook::Call<int(int)>(0x140275FD0)(checksumFeed)) // FS_NeedRestart
    {
      return false;
    }

    FsRestartSync(localClientNum, checksumFeed);
    return true;
  }

  // Note that x86 restarts the file system in CL_Vid_Restart_f() before it
  // updates the language, so that a mod or a language picked from the menu
  // takes effect, and later reloads the stats of the mod if it did (see
  // FileSystem.asm). x64 leaves the restart out.
  //
  void FileSystem::CL_Vid_Restart_f_UpdateLanguageInfo_Hk()
  {
    AssertOffset(Game::clientConnection_t, checksumFeed, 0x130);

    FileSystem_VidRestarted = FS_ConditionalRestart(0, Game::clientConnections[0].checksumFeed);

    Utils::Hook::Call<void()>(0x14024D840)(); // SEH_UpdateLanguageInfo
  }

  // Note that FS_Shutdown() takes no argument on x64.
  //
  void FileSystem::FsShutdownSync()
  {
    std::lock_guard _(FSMutex);
    Maps::GetUserMap()->freeIwd();
    Utils::Hook::Call<void()>(0x140276A30)(); // FS_Shutdown
  }

  void FileSystem::DelayLoadImagesSync()
  {
    std::lock_guard _(FSMutex);
    return Utils::Hook::Call<void()>(0x14012EA90)(); // DB_LoadDelayedImages
  }

  int FileSystem::LoadTextureSync(Game::GfxImageLoadDef** loadDef, Game::GfxImage* image)
  {
    std::lock_guard _(FSMutex);
    return Game::Load_Texture(loadDef, image);
  }

  void FileSystem::IwdFreeStub(Game::iwd_t* iwd)
  {
    Maps::GetUserMap()->handlePackfile(iwd);
    Utils::Hook::Call<void(void*)>(0x14027DD10)(iwd); // free
  }

  const char* FileSystem::Sys_DefaultInstallPath_Hk()
  {
    static auto current_path = std::filesystem::current_path().string();
    return current_path.data();
  }

  FILE* FileSystem::FS_FileOpenReadText_Hk(const char* file)
  {
    const auto path = Utils::GetBaseFilesLocation();
    if (!path.empty() && Utils::IO::FileExists((path / file).string()))
    {
      return Game::FS_FileOpenReadText((path / file).string().data());
    }

    return Game::FS_FileOpenReadText(file);
  }

  // Look up localized files in other languages if the current language
  // doesn't have them.
  //
  // The search path has the localized iwds of every language we have but the
  // lookup only considers those of the current language. The localized zones,
  // however, are those of the installation's language (see Win_GetLanguage())
  // and ours fall back to English if we don't have them in that language (see
  // FastFiles::LoadLocalizeZones()). So, say, with an English installation
  // switched to Japanese, the zones refer to images that are only in the
  // English iwds and the game fails to find them.
  //
  // So if the lookup fails, we retry it with the installation's language and
  // then English in place of the current language for the localized iwds.
  // Note that this way the current language still takes precedence.
  //
  // Note also that files are looked up from several threads (for example,
  // images are loaded from the database thread), thus thread-local.
  //
  static thread_local int fallback_language (-1);

  // Return the languages to look up localized files in, in order.
  //
  static std::vector<int>
  lookup_languages ()
  {
    std::vector<int> r {Game::SEH_GetCurrentLanguage ()};

    auto add = [&r] (int l)
    {
      if (std::ranges::find (r, l) == r.end ())
        r.push_back (l);
    };

    if (const char* n = Game::Win_GetLanguage ())
    {
      auto ls (std::views::iota (0, static_cast<int> (Game::MAX_LANGUAGES)));
      auto i (std::ranges::find_if (ls,
                                    [n] (int l)
      {
        return std::strcmp (Game::SEH_GetLanguageName (l), n) == 0;
      }));

      if (i != ls.end ())
        add (*i);
    }

    add (Game::LANGUAGE_ENGLISH);
    return r;
  }

  int FileSystem::
  FS_FOpenFileReadForThread_Hk (const char* filename, Game::fileHandle_t* file, int thread)
  {
    int r (FS_FOpenFileReadForThread_Original (filename, file, thread));

    if (r == -1)
    {
      for (int l: lookup_languages () | std::views::drop (1))
      {
        fallback_language = l;
        r = FS_FOpenFileReadForThread_Original (filename, file, thread);
        fallback_language = -1;

        if (r != -1)
          break;
      }
    }

    return r;
  }

  int FileSystem::
  SEH_GetCurrentLanguage_Hk ()
  {
    return fallback_language != -1
      ? fallback_language
      : Game::SEH_GetCurrentLanguage ();
  }

  // Explain why an image couldn't be loaded if it is missing.
  //
  // All the game says is that it couldn't load the image, which tells the
  // player nothing. So if that's because its file is nowhere to be found (in
  // any of the languages we look it up in, see above), we say so as well as
  // what is the likely cause. Otherwise (the file is truncated, etc), we keep
  // the original error.
  //
  void FileSystem::
  Image_LoadError (Game::errorParm_t error, const char* fmt, const char* name)
  {
    std::string f (std::format ("images/{}.iwi", name));

    if (Game::FS_FOpenFileReadForThread (f.c_str (), nullptr, Game::FS_THREAD_MAIN) != -1)
    {
      Game::Com_Error (error, fmt, name);
      return;
    }

    std::string ls;
    {
      std::vector<int> v (lookup_languages ());
      for (std::size_t i (0); i != v.size (); ++i)
      {
        if (i != 0)
          ls += (i + 1 == v.size () ? " and " : ", ");

        ls += Game::SEH_GetLanguageName (v[i]);
      }
    }

    Logger::Error (error,
                   "Couldn't load image '{}': {} is missing for {}. "
                   "Your game files are likely incomplete.",
                   name, f, ls);
  }

  const char* FileSystem::Sys_DefaultCDPath_Hk()
  {
    return Sys_DefaultInstallPath_Hk();
  }

  const char* FileSystem::Sys_HomePath_Hk()
  {
    const auto path = Utils::GetBaseFilesLocation();
    if (!path.empty())
    {
      static auto current_path = path.string();
      return current_path.data();
    }

    return "";
  }

  const char* FileSystem::Sys_Cwd_Hk()
  {
    return Sys_DefaultInstallPath_Hk();
  }

  Game::HunkUser* Hunk_UserCreate_Stub(int maxSize, const char* name, bool fixed, int type)
  {
    maxSize *= FILE_COUNT_MULTIPLIER;
    return Utils::Hook::Call<Game::HunkUser*(int, const char*, bool, int)>(0x14027D390)(maxSize, name, fixed, type);
  }

  bool FileSystem::FileWrapper_Rotate(const char* ospath)
  {
    constexpr auto MAX_BACKUPS = 20;

    std::string renamedPath;

    std::optional<int> oldestIndex;
    auto currentIndex = 0;
    std::filesystem::file_time_type oldestime{};

    // Check if the original file exists
    if (!Utils::IO::FileExists(ospath))
    {
      return true; // Return true if the file does not exist (no file to rotate)
    }

    for (; currentIndex < MAX_BACKUPS; ++currentIndex)
    {
      renamedPath = std::format("{0}.{1:03}", ospath, currentIndex);

      if (!Utils::IO::FileExists(renamedPath))
      {
        break; // Stop if an available slot is found
      }

      auto time = std::filesystem::last_write_time(renamedPath);
      if (!oldestIndex.has_value() || time < oldestime)
      {
        oldestime = time;
        oldestIndex = currentIndex;
      }
    }

    if (currentIndex == MAX_BACKUPS)
    {
      renamedPath = std::format("{0}.{1:03}", ospath, *oldestIndex);
      Utils::IO::RemoveFile(renamedPath); // Remove the oldest backup file
    }
    else
    {
      renamedPath = std::format("{0}.{1:03}", ospath, currentIndex);
    }

    // Rename the original file to the selected backup slot
    std::error_code ec;
    std::filesystem::rename(ospath, renamedPath, ec);

    return !ec;
  }

  bool FileSystem::FileRotate(const std::string& filename)
  {
    std::array<char, MAX_OSPATH> ospath{};

    const auto* basepath = (*Game::fs_homepath)->current.string;
    Game::FS_BuildOSPath(basepath, Game::fs_gamedir, filename.c_str(), ospath.data());
    return FileWrapper_Rotate(ospath.data());
  }

  FileSystem::FileSystem()
  {
    // Thread safe file system interaction
    Utils::Hook(0x140277607, AllocateFile, HOOK_CALL).install()->quick();
    Utils::Hook(Game::FS_FreeFile, FreeFile, HOOK_JUMP).install()->quick();

    // Filesystem config checks
    Utils::Hook(0x1401E7220, Cmd_Exec_f_Stub, HOOK_CALL).install()->quick();

    // Don't strip the folders from the config name (otherwise our ExecIsFSStub fails)
    //
    // Note that on x64 the stripped name comes back in rax and is then moved
    // into the argument register, so we also drop the move, which leaves the
    // full name there.
    //
    Utils::Hook::Nop(0x1401E7211, 5);
    Utils::Hook::Nop(0x1401E721D, 3);

    // Register additional folders
    //
    // Note that we hook FS_Startup() where it goes on to the directories of
    // fs_game, which it starts by loading the dvar into rax. No volatile
    // register is live there, so the stub only repeats that load before it
    // jumps back.
    //
    Utils::Hook(0x14027706C, FileSystem_StartupStub, HOOK_JUMP).install()->quick();

    // exec whitelist removal
    //
    // Note that x64 inlines the check of the command against the list, so
    // here we make the jump it takes on a match unconditional.
    //
    Utils::Hook::Set<std::uint8_t>(0x1401E76C6, 0xEB);

    // ignore 'no iwd files found in main'
    Utils::Hook::Nop(0x1402732AF, 5);

    // Ignore bad magic, when trying to free hunk when it's already cleared
    //
    // Note that on x64 we replace the error with a jump to the epilogue of
    // the function.
    //
    Utils::Hook(0x14027DCA2, 0x14027DD09, HOOK_JUMP).install()->quick();

    // Synchronize filesystem starts
    Utils::Hook(0x140275362, FsStartupSync, HOOK_CALL).install()->quick(); // FS_InitFilesystem
    Utils::Hook(0x1402765CF, FsStartupSync, HOOK_CALL).install()->quick(); // FS_Restart

    // Synchronize filesystem restarts
    //
    // Note that x64 drops FS_ConditionalRestart(), whose call we hook on
    // x86. It also restarts the file system when the host starts a party
    // match, which x86 doesn't.
    //
    Utils::Hook(0x140239802, FsRestartSync, HOOK_CALL).install()->quick(); // SV_SpawnServer
    Utils::Hook(0x140100308, FsRestartSync, HOOK_CALL).install()->quick(); // CL_ParseServerMessage
    Utils::Hook(0x140111FCF, FsRestartSync, HOOK_CALL).install()->quick(); // StartMatch

    // Restart the file system on vid_restart as x86 does, and reload the
    // stats of the mod if we did. See FileSystem.asm for the registers at
    // the second hook point, the 'cmp dword_140C5AE68, r15d' before the
    // reload.
    //
    Utils::Hook(0x1400FDD86, CL_Vid_Restart_f_UpdateLanguageInfo_Hk, HOOK_CALL).install()->quick(); // CL_Vid_Restart_f
    Utils::Hook(0x1400FDF36, FileSystem_VidRestartedStub, HOOK_CALL).install()->quick(); // CL_Vid_Restart_f
    Utils::Hook::Nop(0x1400FDF3B, 2);

    // Synchronize filesystem stops
    //
    // Note that x64 inlines FS_Shutdown() into FS_Restart(), where we hook
    // its call on x86. Every caller of FS_Restart() goes through
    // FsRestartSync(), which already holds the lock and frees the iwd of the
    // user map, so there is nothing left to hook in it.
    //
    Utils::Hook(0x1401F58D1, FsShutdownSync, HOOK_CALL).install()->quick(); // Com_Quitf

    // Synchronize db image loading
    Utils::Hook(0x140117F41, DelayLoadImagesSync, HOOK_CALL).install()->quick();
    Utils::Hook(0x14011AE16, LoadTextureSync, HOOK_CALL).install()->quick();

    // Handle IWD freeing
    //
    // Note that x64 inlines the loop that frees the search paths, whose
    // free() call we hook on x86, into both FS_Shutdown() and FS_Restart().
    //
    Utils::Hook(0x140276ABC, IwdFreeStub, HOOK_CALL).install()->quick(); // FS_Shutdown
    Utils::Hook(0x14027656C, IwdFreeStub, HOOK_CALL).install()->quick(); // FS_Restart

    // Set the working dir based on info from the Xlabs launcher
    Utils::Hook(0x14028C750, Sys_DefaultInstallPath_Hk, HOOK_JUMP).install()->quick();

    // Make the exe run from a folder other than the game folder
    Utils::Hook(0x1402A2BDF, FS_FileOpenReadText_Hk, HOOK_CALL).install()->quick();

    // Note that FS_FOpenFileReadForThread() starts with a 5-byte store of rbx
    // to its home slot.
    //
    // Note also that on x86 the zone builder hooks FS_FOpenFileReadForThread()
    // itself and so we leave it alone when the zone builder is enabled. The
    // zone builder isn't part of the x64 build.
    //
    FS_FOpenFileReadForThread_Original = Utils::Hook::Detour (0x140274100, 5, &FS_FOpenFileReadForThread_Hk);
    Utils::Hook (0x1402741B1, SEH_GetCurrentLanguage_Hk, HOOK_CALL).install ()->quick (); // FS_FOpenFileReadForThread: SEH_GetCurrentLanguage().
    Utils::Hook (0x14027448E, SEH_GetCurrentLanguage_Hk, HOOK_CALL).install ()->quick (); // FS_FOpenFileReadForThread: SEH_GetCurrentLanguage().

    Utils::Hook (0x140037AC3, Image_LoadError, HOOK_CALL).install ()->quick (); // Delayed image load: Com_Error().
    Utils::Hook (0x140037BC9, Image_LoadError, HOOK_CALL).install ()->quick (); // Load_Texture: Com_Error().

    // Make the exe run from a folder other than the game folder
    //
    // Note that x64 inlines FS_IsBasePathValid() into FS_InitFilesystem(),
    // so here we jump past both the check and its error.
    //
    Utils::Hook(0x14027536C, 0x1402753B4, HOOK_JUMP).install()->quick(); // FS_IsBasePathValid
    // ^^ This check by the game above is super redundant, IW4x has other checks in place to make sure we
    // are running from a properly installed directory. This only breaks the containerized patch and we don't need it

    // Patch FS dvar values
    Utils::Hook(0x140276332, Sys_DefaultCDPath_Hk, HOOK_CALL).install()->quick();
    Utils::Hook(0x14027647E, Sys_HomePath_Hk, HOOK_CALL).install()->quick();
    Utils::Hook(0x14027635A, Sys_Cwd_Hk, HOOK_CALL).install()->quick();
    Utils::Hook(0x140035C5F, Sys_Cwd_Hk, HOOK_CALL).install()->quick();

    // patch max file amount returned by Sys_ListFiles
    //
    // Note that x64 inlines Sys_ListFilteredFiles() into Sys_ListFiles() and
    // drops its check of the count on entry, where the count is always 0.
    // The file list also holds 8-byte pointers on x64.
    //
    Utils::Hook::Set<std::uint32_t>(0x14028C95F + 1, static_cast<std::uint32_t>((NEW_MAX_FILES_LISTED + FILE_COUNT_MULTIPLIER) * sizeof(const char*)));
    Utils::Hook::Set<std::uint32_t>(0x14028CA40 + 3, NEW_MAX_FILES_LISTED);
    Utils::Hook::Set<std::uint32_t>(0x14028CCB7 + 3, NEW_MAX_FILES_LISTED);

    // Sys_ListFiles
    Utils::Hook(0x14028C99C, Hunk_UserCreate_Stub, HOOK_CALL).install()->quick();
    Utils::Hook(0x14028CBE5, Hunk_UserCreate_Stub, HOOK_CALL).install()->quick();

    // FS_ListFilteredFiles
    //
    // Note that x64 also inlines a copy of FS_AddFileToList() into
    // FS_ListFilteredFiles(), and the copy has a count check of its own.
    //
    Utils::Hook(0x140275592, Hunk_UserCreate_Stub, HOOK_CALL).install()->quick();
    Utils::Hook::Set<std::uint32_t>(0x140272DCA + 2, NEW_MAX_FILES_LISTED); // FS_AddFileToList
    Utils::Hook::Set<std::uint32_t>(0x140275810 + 3, NEW_MAX_FILES_LISTED);
    Utils::Hook::Set<std::uint32_t>(0x140275597 + 1, static_cast<std::uint32_t>((NEW_MAX_FILES_LISTED + FILE_COUNT_MULTIPLIER) * sizeof(const char*) + sizeof(const char*)));
  }

  FileSystem::~FileSystem()
  {
    assert(FileSystem::MemAllocator.empty());
  }
}

bool FileSystem_VidRestarted = false;

void FileSystem_RegisterFolders()
{
  Components::FileSystem::RegisterFolders();
}
