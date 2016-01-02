#include "..\..\STDInclude.hpp"

namespace Components
{
	void FileSystem::File::Read()
	{
		char* buffer = nullptr;
		int size = Game::FS_ReadFile(this->FilePath.data(), &buffer);

		this->Buffer.clear();

		if (size < 0)
		{
			if (buffer)
			{
				Game::FS_FreeFile(buffer);
			}
		}
		else
		{
			this->Buffer.append(buffer, size);
			Game::FS_FreeFile(buffer);
		}
	}

	int FileSystem::ExecIsFSStub(const char* execFilename)
	{
		return !File(execFilename).Exists();
	}

	FileSystem::FileSystem()
	{
		// Filesystem config checks
		Utils::Hook(0x6098FD, FileSystem::ExecIsFSStub, HOOK_CALL).Install()->Quick();

		// exec whitelist removal (YAYFINITY WARD)
		Utils::Hook::Nop(0x609685, 5);
		Utils::Hook::Nop(0x60968C, 2);
	}
}
