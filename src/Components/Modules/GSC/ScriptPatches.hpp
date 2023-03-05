#pragma once

namespace Components::GSC
{
	class ScriptPatches : public Component
	{
	public:
		ScriptPatches();

	private:
		static Game::game_hudelem_s* HECmd_GetHudElem(Game::scr_entref_t entref);

		static void Scr_TableLookupIStringByRow_Hk();
	};
}
