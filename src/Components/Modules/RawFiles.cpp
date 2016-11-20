#include "STDInclude.hpp"

namespace Components
{
	void* RawFiles::LoadModdableRawfileFunc(const char* filename)
	{
		return Game::LoadModdableRawfile(0, filename);
	}

	RawFiles::RawFiles()
	{
		Utils::Hook(0x632155, RawFiles::LoadModdableRawfileFunc, HOOK_CALL).install()->quick();
		Utils::Hook(0x5FA46C, RawFiles::LoadModdableRawfileFunc, HOOK_CALL).install()->quick();
		Utils::Hook(0x5FA4D6, RawFiles::LoadModdableRawfileFunc, HOOK_CALL).install()->quick();
		Utils::Hook(0x6321EF, RawFiles::LoadModdableRawfileFunc, HOOK_CALL).install()->quick();
		Utils::Hook(0x630A88, RawFiles::LoadModdableRawfileFunc, HOOK_CALL).install()->quick();
		Utils::Hook(0x59A6F8, RawFiles::LoadModdableRawfileFunc, HOOK_CALL).install()->quick();
		Utils::Hook(0x57F1E6, RawFiles::LoadModdableRawfileFunc, HOOK_CALL).install()->quick();
		Utils::Hook(0x57ED36, RawFiles::LoadModdableRawfileFunc, HOOK_CALL).install()->quick();

		// remove fs_game check for moddable rawfiles - allows non-fs_game to modify rawfiles
		Utils::Hook::Nop(0x61AB76, 2);

		Command::Add("dumpraw", [] (Command::Params params)
		{
			if (params.length() < 2)
			{
				Logger::Print("Specify a filename!\n");
				return;
			}

			FileSystem::File file(params.join(1));
			if (file.exists())
			{
				Utils::IO::WriteFile("raw/" + file.getName(), file.getBuffer());
				Logger::Print("File '%s' written to raw!\n", file.getName().data());
				return;
			}

			const char* data = Game::LoadModdableRawfile(0, file.getName().data());

			if (data)
			{
				Utils::IO::WriteFile("raw/" + file.getName(), data);
				Logger::Print("File '%s' written to raw!\n", file.getName().data());
			}
			else
			{
				Logger::Print("File '%s' does not exist!\n", file.getName().data());
			}
		});
	}
}
