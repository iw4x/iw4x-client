#include "Entity.hpp"
#include "Script.hpp"

namespace Components::GSC
{
	void Entity::AddScriptMethods()
	{
		Script::AddMethod("SetBrushModel", [](const Game::scr_entref_t entref) // gsc: <entity> SetBrushModel(<index>)
		{
			if (Game::Scr_GetNumParam() != 1)
			{
				Game::Scr_Error("usage: <entity> SetBrushModel( <index> )\n");
				return;
			}

			auto* ent = Game::GetEntity(entref);
			const auto index = Game::Scr_GetInt(0);

			if (index < 0 || static_cast<unsigned int>(index) >= Game::cm->numSubModels)
			{
				Game::Scr_ParamError(0, "brush model index out of range");
				return;
			}

			Game::SV_UnlinkEntity(ent);
			ent->s.index.brushModel = index;
			ent->r.modelType = Game::MODELTYPE_BRUSH;

			Game::SV_SetBrushModel(ent);
			Game::SV_LinkEntity(ent);
		});
	}

	Entity::Entity()
	{
		AddScriptMethods();
	}
}
