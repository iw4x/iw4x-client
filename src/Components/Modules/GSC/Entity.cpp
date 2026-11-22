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
      const auto tagName = Utils::Hook::Call<unsigned int(unsigned int)>(0x140228100)(0); // Scr_GetConstLowercaseString

      // On x86 we ask the cache of G_DObjGetWorldTagMatrix() for the bone,
      // but x64 inlines that function and its cache with it. So we look the
      // bone up in the entity's DObj ourselves, which is what the cache does
      // when it misses anyway. It's probably a bit slower without the cache,
      // but TagExists isn't something scripts call every frame. Note that 0xFF
      // would mean that we already know the bone is missing, so we start from
      // another invalid index.
      //
      const auto* dobj = Utils::Hook::Call<void*(Game::gentity_s*)>(0x1401FB9D0)(ent); // Com_GetServerDObj
      unsigned char boneIndex = 0xFE;

      const auto exists = dobj && Utils::Hook::Call<int(const void*, unsigned int, unsigned char*)>(0x1402AA690)(dobj, tagName, &boneIndex); // DObjGetBoneIndex
      Game::Scr_AddBool(exists);
    });
  }

  Entity::Entity()
  {
    AddScriptMethods();
  }
}
