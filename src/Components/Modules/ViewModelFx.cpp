#include <STDInclude.hpp>
#include "Components/Modules/GSC/Script.hpp"
#include "Components/Modules/GSC/ScriptExtension.hpp"
#include "Components/Modules/Weapon.hpp"
#include "Components/Modules/ViewModelFxSetup.hpp"

namespace Components::ViewModelFxSetup
{
	static char cg_weaponsArray[32 * Weapon::WEAPON_LIMIT];

	void Add_GSC_Functions()
	{
		// 3arc function for iw game
		GSC::Script::AddFunction("PlayViewmodelFX", []
		{
			if (Game::Scr_GetNumParam() != 2)
			{
				Game::Scr_Error("PlayViewmodelFX() called with wrong params.\n");
				return;
			}

			const char* fxName = Game::Scr_GetString(0); // FX name string
			Game::scr_string_t tagName = Game::Scr_GetConstString(1); // Bone/tag

			Game::FxEffectDef* fx = Game::DB_FindXAssetHeader(Game::ASSET_TYPE_FX, fxName).fx;
			if (!fx)
			{
				Game::Scr_Error(Utils::String::VA("PlayViewmodelFX(): FX '%s' not found", fxName));
				return;
			}

			int clientNum = 0; // local player
			const Game::DObj* dobj = Game::Com_GetClientDObj(clientNum, 0);
			if (!dobj)
			{
				Game::Scr_Error("PlayViewmodelFX(): Could not get DObj for local player");
				return;
			}

			unsigned char boneIdx = 0;
			if (!Game::DObjGetBoneIndex(reinterpret_cast<int>(dobj), tagName, &boneIdx))
			{
				Game::Scr_Error(Utils::String::VA(
					"PlayViewmodelFX(): clientNum '%d' does not have bone '%s'",
					clientNum,
					Game::SL_ConvertToString(tagName)
				));
				return;
			}

			int dobjHandle = dobj->entnum;
			Game::CG_PlayBoltedEffect(0, fx, dobjHandle, tagName);
		});

		GSC::Script::AddFunction("PVMFX_BOTH", []
		{
			if (Game::Scr_GetNumParam() != 2)
			{
				Game::Scr_Error("PVMFX_BOTH() called with wrong params. Expected 2.\n");
				return;
			}

			const char* fxName = Game::Scr_GetString(0); // FX name string
			Game::scr_string_t tagName = Game::Scr_GetConstString(1); // Bone/tag

			Game::FxEffectDef* fx = Game::DB_FindXAssetHeader(Game::ASSET_TYPE_FX, fxName).fx;
			if (!fx)
			{
				Game::Scr_Error(Utils::String::VA("PVMFX_BOTH(): FX '%s' not found", fxName));
				return;
			}

			for (int hand = 0; hand <= 1; ++hand)
			{
				int fpDObjHandle = Game::CG_WeaponDObjHandle(hand);
				if (fpDObjHandle)
				{
					Game::CG_PlayBoltedEffect(0, fx, fpDObjHandle, tagName);

					Game::CG_StopBoltedEffect(0, 0, fpDObjHandle, tagName);
				}
			}

			int clientNum = 0;
			const Game::DObj* tpDObj = Game::Com_GetClientDObj(clientNum, 0);
			if (tpDObj)
			{
				Game::CG_PlayBoltedEffect(0, fx, tpDObj->entnum, tagName);

				Game::CG_StopBoltedEffect(0, 0, tpDObj->entnum, tagName);
			}
		});

		GSC::Script::AddMethod("setanim", [](Game::scr_entref_t entref)
		{
			auto* ent = GSC::Script::Scr_GetPlayerEntity(entref);
			if (!ent || !ent->client)
				return;


			int anim = Game::Scr_GetInt(0);

			for (int i = 0; i < 2; i++)
				ent->client->ps.weapState[i].weapAnim = anim;
		});
	}

	Setup::Setup()
	{
		Add_GSC_Functions();
	}
}
