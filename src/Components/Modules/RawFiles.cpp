#include "STDInclude.hpp"

namespace Components
{
	void* RawFiles::LoadModdableRawfileFunc(const char* filename)
	{
		return Game::LoadModdableRawfile(0, filename);
	}

	RawFiles::RawFiles()
	{
		Utils::Hook(0x632155, RawFiles::LoadModdableRawfileFunc, HOOK_CALL).Install()->Quick();
		Utils::Hook(0x5FA46C, RawFiles::LoadModdableRawfileFunc, HOOK_CALL).Install()->Quick();
		Utils::Hook(0x5FA4D6, RawFiles::LoadModdableRawfileFunc, HOOK_CALL).Install()->Quick();
		Utils::Hook(0x6321EF, RawFiles::LoadModdableRawfileFunc, HOOK_CALL).Install()->Quick();
		Utils::Hook(0x630A88, RawFiles::LoadModdableRawfileFunc, HOOK_CALL).Install()->Quick();
		Utils::Hook(0x59A6F8, RawFiles::LoadModdableRawfileFunc, HOOK_CALL).Install()->Quick();
		Utils::Hook(0x57F1E6, RawFiles::LoadModdableRawfileFunc, HOOK_CALL).Install()->Quick();
		Utils::Hook(0x57ED36, RawFiles::LoadModdableRawfileFunc, HOOK_CALL).Install()->Quick();

		// remove fs_game check for moddable rawfiles - allows non-fs_game to modify rawfiles
		Utils::Hook::Nop(0x61AB76, 2);

		// This is placed here in case the anticheat has been disabled!
#ifndef DEBUG
		QuickPatch::OnFrame(AntiCheat::FlagIntegrityCheck);
#endif

		Command::Add("dumpraw", [] (Command::Params params)
		{
			if (params.Length() < 2)
			{
				Logger::Print("Specify a filename!\n");
				return;
			}

			std::string filename = params[1];
			const char* data = Game::LoadModdableRawfile(0, filename.data());

			if (data)
			{
				Utils::IO::WriteFile("raw/" + filename, data);
				Logger::Print("File '%s' written to raw!\n", filename.data());
			}
			else
			{
				Logger::Print("File '%s' does not exist!\n", filename.data());
			}
		});
	}
}
