#include <STDInclude.hpp>
#include "ScriptPatches.hpp"
#include "Script.hpp"

using namespace Utils::String;

namespace Components
{
	constexpr auto offset = 511;

	Game::game_hudelem_s* ScriptPatches::HECmd_GetHudElem(Game::scr_entref_t entref)
	{
		if (entref.classnum != 1)
		{
			Game::Scr_ObjectError("not a hud element");
			return nullptr;
		}

		assert(entref.entnum < 1024);
		return &Game::g_hudelems[entref.entnum];
	}

	ScriptPatches::ScriptPatches()
	{
		Script::AddMethod("ClearHudText", [](Game::scr_entref_t entref) -> void
		{
			auto* hud = HECmd_GetHudElem(entref);

			// Frees config string up
			if ((hud->elem).text)
			{
				Game::SV_SetConfigstring((hud->elem).text + offset, nullptr);
			}
		});
	}
}
