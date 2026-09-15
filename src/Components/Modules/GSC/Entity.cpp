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

		Script::AddMethod("TagExists", [](const Game::scr_entref_t entref)
		{
			if (Game::Scr_GetNumParam() != 1)
			{
				Game::Scr_Error("usage: <entity> TagExists( <tag name> )\n");
				return;
			}

			auto* ent = Game::GetEntity(entref);
			const auto tagName = Utils::Hook::Call<unsigned int(unsigned int)>(0x47C720)(0);

			const auto exists = Utils::Hook::Call<int(Game::gentity_s*, unsigned int, void*, int)>(0x501120)(ent, tagName, reinterpret_cast<void*>(0x1A860C0), 0);
			Game::Scr_AddBool(exists != 0);
		});
	}

	Entity::Entity()
	{
		AddScriptMethods();
	}
}
