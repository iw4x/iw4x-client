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

	FileSystem::FileSystem()
	{

	}
}
