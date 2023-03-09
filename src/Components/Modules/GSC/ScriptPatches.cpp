#include <STDInclude.hpp>
#include "ScriptPatches.hpp"
#include "Script.hpp"

using namespace Utils::String;

namespace Components::GSC
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

	void ScriptPatches::Scr_TableLookupIStringByRow_Hk()
	{
		if (Game::Scr_GetNumParam() < 3)
		{
			Game::Scr_Error("USAGE: tableLookupIStringByRow( filename, rowNum, returnValueColumnNum )");
			return;
		}

		const auto* fileName = Game::Scr_GetString(0);
		const auto rowNum = Game::Scr_GetInt(1);
		const auto returnValueColumnNum = Game::Scr_GetInt(2);

		const auto* table = Game::DB_FindXAssetHeader(Game::ASSET_TYPE_STRINGTABLE, fileName).stringTable;

		if (!table)
		{
			Game::Scr_ParamError(0, Utils::String::VA("%s does not exist", fileName));
			return;
		}

		const auto* value = Game::StringTable_GetColumnValueForRow(table, rowNum, returnValueColumnNum);
		Game::Scr_AddIString(value);
	}

	ScriptPatches::ScriptPatches()
	{
		// Fix format string in Scr_RandomFloatRange
		Utils::Hook::Set<const char*>(0x5F10C6, "Scr_RandomFloatRange parms: %f %f ");

		// Correct builtin function pointer
		Utils::Hook::Set<Game::BuiltinFunction>(0x79A90C, Scr_TableLookupIStringByRow_Hk);

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
