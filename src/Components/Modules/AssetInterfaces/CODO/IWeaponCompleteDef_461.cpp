#include <STDInclude.hpp>
#include "IWeaponCompleteDef_461.hpp"

	IWeaponCompleteDef_461::WeaponCompleteDef IWeaponCompleteDef_461::Read461WeaponCompleteDef()
	{
#define LOAD_XSTRING(member)\
		*Game::varXString = reinterpret_cast<char**>(&member);\
		Game::Load_XString(false)

#define LOAD_MATERIAL(member)\
		*Game::varMaterialHandle = reinterpret_cast<Game::Material**>(&member);\
		Game::Load_MaterialHandle(false)

#define LOAD_XSTRING_ARRAY(member, count)\
		*Game::varXString = reinterpret_cast<char**>(&member);\
		Game::Load_XStringArray(false, count)

#define LOAD_FX(member)\
		*Game::varFxEffectDefHandle = &member;\
		Game::Load_FxEffectDefHandle(false);

#define LOAD_SOUND(member)\
		{\
			*Game::varsnd_alias_list_name = &member;\
			Game::Load_SndAliasCustom(*Game::varsnd_alias_list_name);\
		}


		IWeaponCompleteDef_461::WeaponCompleteDef target{};

		Game::Load_Stream(true, reinterpret_cast<void*>(&target), WEAPON_ASSET_SIZE);
		Game::DB_PushStreamPos(3);

		LOAD_XSTRING(target.szInternalName);
		LOAD_XSTRING(target.szDisplayName);
		LOAD_XSTRING(target.szInternalName2);
		LOAD_XSTRING(target.unkString3);

		for (size_t i = 0; i < ARRAYSIZE(target.unkModels1); i++)
		{
			*Game::varXModelPtr = &target.unkModels1[i];
			Game::Load_XModelPtr(false);
		}

		for (size_t i = 0; i < ARRAYSIZE(target.unkModels2); i++)
		{
			*Game::varXModelPtr = &target.unkModels2[i];
			Game::Load_XModelPtr(false);
		}

		for (size_t i = 0; i < ARRAYSIZE(target.unkModels_Multiple1); i++)
		{
			*Game::varXModelPtr = &target.unkModels_Multiple1[i];
			Game::Load_XModelPtr(false);
		}

		LOAD_XSTRING_ARRAY(target.unkXStringArray1, 62);
		LOAD_XSTRING_ARRAY(target.unkXStringArray2, 62);
		LOAD_XSTRING_ARRAY(target.unkXStringArray3, 62);

		if (target.szInternalName == "ak47holiday_mp"s)
		{
			printf("");
		}

		std::string test(target.szInternalName);

		for (size_t i = 0; i < 16; i++)
		{
			*Game::varFxEffectDefHandle = &target.effects[i];
			Game::Load_FxEffectDefHandle(false);
		}

		// <Skip unkScriptString5>

		for (size_t i = 0; i < 16; i++)
		{
			*Game::varFxEffectDefHandle = &target.effects2[i];
			Game::Load_FxEffectDefHandle(false);
		}

		

		for (size_t i = 0; i < 16; i++)
		{
			*Game::varFxEffectDefHandle = &target.effects2b[i];
			Game::Load_FxEffectDefHandle(false);
		}

		

		for (size_t i = 0; i < 16; i++)
		{
			*Game::varFxEffectDefHandle = &target.effects3[i];
			Game::Load_FxEffectDefHandle(false);
		}

		

		*Game::varFxEffectDefHandle = &target.viewFlashEffect;
		Game::Load_FxEffectDefHandle(false);

		

		*Game::varFxEffectDefHandle = &target.worldFlashEffect;
		Game::Load_FxEffectDefHandle(false);

		

		*Game::varFxEffectDefHandle = &target.unkEffect3;
		Game::Load_FxEffectDefHandle(false);

		

		*Game::varFxEffectDefHandle = &target.unkEffect4;
		Game::Load_FxEffectDefHandle(false);

		

		*Game::varFxEffectDefHandle = &target.unkEffect5;
		Game::Load_FxEffectDefHandle(false);

		

		LOAD_XSTRING(target.unkString4);
		LOAD_XSTRING(target.unkString5);

		for (size_t i = 0; i < ARRAYSIZE(target.sounds); i++)
		{
			*Game::varsnd_alias_list_name = &target.sounds[i];
			Game::Load_SndAliasCustom(*Game::varsnd_alias_list_name);
		}

		

		for (size_t i = 0; i < ARRAYSIZE(target.sounds1b); i++)
		{
			*Game::varsnd_alias_list_ptr = &target.sounds1b[i];

			// Load_snd_alias_list_ptr
			Utils::Hook::Call<void(bool)>(0x44A870)(false);
		}

		

		if (target.bounceSounds1)
		{
			if (target.bounceSounds1 == reinterpret_cast<void*>(-1))
			{
				target.bounceSounds1 = reinterpret_cast<Game::snd_alias_list_t**>(Game::DB_AllocStreamPos(3));
				*Game::varsnd_alias_list_name = target.bounceSounds1;
				Game::Load_snd_alias_list_nameArray(true, 31);
			}
			else
			{
				Game::DB_ConvertOffsetToPointer(&target.bounceSounds1);
			}
		}

		

		if (target.bounceSounds2)
		{
			target.bounceSounds2 = reinterpret_cast<Game::snd_alias_list_t**>(Game::DB_AllocStreamPos(3));
			*Game::varsnd_alias_list_name = target.bounceSounds2;
			Game::Load_snd_alias_list_nameArray(true, 31);
		}


		LOAD_FX(target.viewShellEjectEffect);

		LOAD_FX(target.worldShellEjectEffect);

		LOAD_FX(target.viewLastShotEjectEffect);

		LOAD_FX(target.worldLastShotEjectEffect);

		LOAD_MATERIAL(target.reticleCenter);

		LOAD_MATERIAL(target.reticleSide);

		for (size_t i = 0; i < ARRAYSIZE(target.materials); i++)
		{
			*Game::varMaterialHandle = reinterpret_cast<Game::Material**>(&target.materials[i]);
			Game::Load_MaterialHandle(false);
		}

		

		LOAD_MATERIAL(target.material3);

		

		LOAD_MATERIAL(target.material4);

		LOAD_MATERIAL(target.overlayMaterial);
		LOAD_MATERIAL(target.overlayMaterialLowRes);
		LOAD_MATERIAL(target.overlayMaterialEMP);
		LOAD_MATERIAL(target.overlayMaterialEMPLowRes);

		
		LOAD_XSTRING(target.unkString6);
		LOAD_XSTRING(target.unkString7);
		LOAD_XSTRING(target.unkString8);


		for (size_t i = 0; i < ARRAYSIZE(target.unkMaterials2); i++)
		{
			*Game::varMaterialHandle = reinterpret_cast<Game::Material**>(&target.unkMaterials2[i]);
			Game::Load_MaterialHandle(false);
		}

		

		*Game::varPhysCollmapPtr = reinterpret_cast<Game::PhysCollmap**>(&target.physCollMap);
		Game::Load_PhysCollmapPtr(false);

		

		*Game::varPhysPresetPtr = reinterpret_cast<Game::PhysPreset**>(&target.physPreset);
		Game::Load_PhysPresetPtr(false);

		

		*Game::varXModelPtr = &target.projectileModel;
		Game::Load_XModelPtr(false);

		*Game::varFxEffectDefHandle = &target.projExplosionEffect;
		Game::Load_FxEffectDefHandle(false);

		*Game::varFxEffectDefHandle = &target.projDudEffect;
		Game::Load_FxEffectDefHandle(false);

		*Game::varsnd_alias_list_name = &target.projDudSound;
		Game::Load_SndAliasCustom(*Game::varsnd_alias_list_name);

		

		LOAD_SOUND(target.unkSound1);

		

		for (size_t i = 0; i < ARRAYSIZE(target.unkProjEffects1); i++)
		{
			*Game::varFxEffectDefHandle = &target.unkProjEffects1[i];
			Game::Load_FxEffectDefHandle(false);
		}

		

		*Game::varsnd_alias_list_name = &target.unkSound2;
		Game::Load_SndAliasCustom(*Game::varsnd_alias_list_name);

		

		*Game::varFxEffectDefHandle = &target.projTrailEffect;
		Game::Load_FxEffectDefHandle(false);

		

		*Game::varFxEffectDefHandle = &target.projBeaconEffect;
		Game::Load_FxEffectDefHandle(false);

		

		for (size_t i = 0; i < ARRAYSIZE(target.unkProjEffects2); i++)
		{
			*Game::varFxEffectDefHandle = &target.unkProjEffects2[i];
			Game::Load_FxEffectDefHandle(false);
		}

		LOAD_XSTRING(target.accuracyGraphName1);

		// Mysterious assets ?
		const auto read_mysterious_uint64_asset = [](uint64_t** ptr, unsigned short count)
		{
			const auto size = count * 8;

			if (*ptr == reinterpret_cast<uint64_t*>(-1))
			{
				*ptr = reinterpret_cast<uint64_t*>(Game::DB_AllocStreamPos(3));
				if (size)
				{
					if (*Game::g_streamPosIndex == 2)
					{
						*ptr = nullptr;
					}
					else
					{
						Game::DB_ReadXFile(*ptr, size);
					}

					*Game::g_streamPos += size;
				}
			}
			else
			{
				*ptr = nullptr;
			}
		};

		read_mysterious_uint64_asset(&target.unknownAsset1, target.numberOfUnknownAsset1);

		

		// Out of order
		LOAD_XSTRING(target.accuracyGraphName2);

		

		read_mysterious_uint64_asset(&target.unknownAsset2, target.numberOfUnknownAsset2);
		

		LOAD_XSTRING(target.unkString10_b);
		LOAD_XSTRING(target.unkString11);
		LOAD_XSTRING(target.unkString12);
		LOAD_XSTRING(target.unkString13);

		

		for (size_t i = 0; i < ARRAYSIZE(target.unkStrings); i++)
		{
			LOAD_XSTRING(target.unkStrings[i]);
		}

		

		for (size_t i = 0; i < ARRAYSIZE(target.tracers); i++)
		{
			*Game::varTracerDefPtr = &target.tracers[i];
			Game::Load_TracerDefPtr(false);
		}

		

		LOAD_SOUND(target.unkSound3);

		

		LOAD_FX(target.unkEffect6);

		

		LOAD_XSTRING(target.unkString14);

		

		LOAD_SOUND(target.sounds2);

		

		*Game::varsnd_alias_list_ptr = target.turretBarrelSpinUpSnd;
		Game::Load_snd_alias_list_nameArray(false, 4);

		*Game::varsnd_alias_list_ptr = target.turretBarrelSpinDownSnd;
		Game::Load_snd_alias_list_nameArray(false, 4);

		for (size_t i = 0; i < ARRAYSIZE(target.sounds3); i++)
		{
			LOAD_SOUND(target.sounds3[i]);
		}

		

		for (size_t i = 0; i < ARRAYSIZE(target.sounds4); i++)
		{
			LOAD_SOUND(target.sounds4[i]);
		}

		

		LOAD_XSTRING(target.unkString15);

		

		LOAD_XSTRING(target.unkString16);

		

		LOAD_XSTRING(target.unkString17);

		

		for (size_t i = 0; i < ARRAYSIZE(target.unkMaterials3); i++)
		{
			LOAD_MATERIAL(target.unkMaterials3[i]);
		}

		

		read_mysterious_uint64_asset(&target.unknownAsset3, target.numberOfUnknownAsset1);
		read_mysterious_uint64_asset(&target.unknownAsset4, target.numberOfUnknownAsset2);

		

		for (size_t i = 0; i < ARRAYSIZE(target.unkString18); i++)
		{
			LOAD_XSTRING(target.unkString18[i]);
		}

		LOAD_XSTRING(target.unkString19);

		LOAD_XSTRING(target.unkString20);

		Game::DB_PopStreamPos();

		return target;

#undef LOAD_SOUND
#undef LOAD_XSTRING_ARRAY
#undef LOAD_XSTRING
#undef LOAD_MATERIAL
	}

	void IWeaponCompleteDef_461::Convert461WeaponCompleteDefTo276(const WeaponCompleteDef& definition)
	{
		auto varWeaponCompleteDef = *reinterpret_cast<Game::WeaponCompleteDef**>(0x112A9F4);
		ZeroMemory(varWeaponCompleteDef, sizeof(Game::WeaponCompleteDef));

		varWeaponCompleteDef->szInternalName = definition.szInternalName;
	}
