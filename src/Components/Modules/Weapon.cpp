#include <STDInclude.hpp>
#include "Weapon.hpp"

#include "GSC/Script.hpp"

namespace Components
{
	const Game::dvar_t* Weapon::BGWeaponOffHandFix;
	const Game::dvar_t* Weapon::CGRecoilMultiplier;

	Game::WeaponCompleteDef* Weapon::LoadWeaponCompleteDef(const char* name)
	{
		if (auto* rawWeaponFile = Game::BG_LoadWeaponCompleteDefInternal("mp", name))
		{
			// Fix for rumbles being wrong in the raw files.. This should not happen normally
			// But in effect, it happens very often, so we need to fix it to avoid rumble errors
			{
				if (rawWeaponFile->weapDef->notetrackRumbleMapKeys &&
					rawWeaponFile->weapDef->notetrackRumbleMapKeys[0] == rawWeaponFile->weapDef->notetrackSoundMapKeys[0])
				{
					// This means it's wrong and the dump is bad (it gave sound names as rumble names, which is not possible)
					// The zone has it right so let's take it from there
					const auto zoneWeapon = Game::DB_FindXAssetHeader(Game::ASSET_TYPE_WEAPON, name).weapon;

					if (zoneWeapon)
					{
						// Restore rumble from zone
						std::memcpy(rawWeaponFile->weapDef->notetrackRumbleMapKeys, zoneWeapon->weapDef->notetrackRumbleMapKeys, 16 * sizeof(unsigned short));
						std::memcpy(rawWeaponFile->weapDef->notetrackRumbleMapValues, zoneWeapon->weapDef->notetrackRumbleMapValues, 16 * sizeof(unsigned short));

						rawWeaponFile->weapDef->fireRumble = zoneWeapon->weapDef->fireRumble;
						rawWeaponFile->weapDef->meleeImpactRumble = zoneWeapon->weapDef->meleeImpactRumble;
						rawWeaponFile->weapDef->turretBarrelSpinRumble = zoneWeapon->weapDef->turretBarrelSpinRumble;
					}


					// Oh well. You'll have to fix your weapon file :)
#define VALIDATE(weap, rumble)\
	if (weap->weapDef->rumble && *weap->weapDef->rumble)\
	{\
		std::string path = std::format("rumble/{}", weap->weapDef->rumble);\
		const auto rawfile = Game::DB_FindXAssetHeader(Game::XAssetType::ASSET_TYPE_RAWFILE, path.c_str());\
		if (rawfile.data == nullptr)\
		{\
			Logger::Warning(Game::CON_CHANNEL_FILES, "Weapon {} has INVALID RUMBLE {} '{}'! Remove or replace it.\n", weap->szInternalName, #rumble, weap->weapDef->rumble);\
			weap->weapDef->rumble = nullptr;\
		}\
	}
					VALIDATE(rawWeaponFile, fireRumble);
					VALIDATE(rawWeaponFile, meleeImpactRumble);
					VALIDATE(rawWeaponFile, turretBarrelSpinRumble);

					for (auto i = 0; i < 16; ++i)
					{
						if (!rawWeaponFile->weapDef->notetrackRumbleMapKeys[i])
							break;

						auto noteTrackRumbleMap = rawWeaponFile->weapDef->notetrackRumbleMapValues;
						if (noteTrackRumbleMap[i])
						{
							auto str = Game::SL_ConvertToString(noteTrackRumbleMap[i]);
							if (str && *str)
							{
								std::string path = std::format("rumble/{}", str);
								const auto rawfile = Game::DB_FindXAssetHeader(Game::XAssetType::ASSET_TYPE_RAWFILE, path.c_str());
								if (rawfile.data == nullptr || !strnlen(rawfile.rawfile->buffer, 1))
								{
									// Void it and warn the user
									noteTrackRumbleMap[i] = 0;
									rawWeaponFile->weapDef->notetrackRumbleMapKeys[i] = 0;

									Logger::Warning(
										Game::CON_CHANNEL_FILES,
										"Weapon {} has INVALID RUMBLE {} notetrackRumbleMap #'{}'! Remove or replace it.\n",
										rawWeaponFile->szInternalName,
										str,
										i
									);
								}
							}
						}
					}
#undef VALIDATE
				}
			}

			return rawWeaponFile;
		}

		auto* zoneWeaponFile = Game::DB_FindXAssetHeader(Game::ASSET_TYPE_WEAPON, name).weapon;
		return Game::DB_IsXAssetDefault(Game::ASSET_TYPE_WEAPON, name) ? nullptr : zoneWeaponFile;
	}

	void Weapon::SaveRegisteredWeapons()
	{
		*reinterpret_cast<DWORD*>(0x1A86098) = 0;

		if (Game::BG_GetNumWeapons() > 1u)
		{
			for (unsigned int i = 1; i < Game::BG_GetNumWeapons(); ++i)
			{
				Game::SV_SetConfigstring(i + (i >= BASEGAME_WEAPON_LIMIT ? 2939 : 2804), Game::BG_GetWeaponName(i));
			}
		}
	}

	void Weapon::PatchHintStrings()
	{
		static Game::Material* hintMaterials[WEAPON_LIMIT + 4];

		// CG_RegisterWeapon
		Utils::Hook::Set(0x4EF619, &hintMaterials[4]);

		Utils::Hook::Set(0x58D003, hintMaterials); // CG_DrawCursorhint (All hints related to crosshair, like pickup hints)
		Utils::Hook::Set(0x58969A, &hintMaterials[3]); // hint_health in CG_RegisterGraphics
		Utils::Hook::Set(0x5896AB, &hintMaterials[4]); // hint_friendly in CG_RegisterGraphics

		Utils::Hook::Set<uint32_t>(0x58D01C + 1, ARRAYSIZE(hintMaterials));

		// Has to do with fx, but somehow lies within the material array
		// EDIT: Those are unrelated (PlayFX OnTag, with FX Id, which is also in cg_media but not related to hint materials)
		//Utils::Hook::Set(0x402069, &unknownMaterialArray[32]);
		//Utils::Hook::Set(0x4E05D9, &unknownMaterialArray[32]);

	}

	void Weapon::PatchLimit()
	{
		Utils::Hook::Set<DWORD>(0x403783, WEAPON_LIMIT);
		Utils::Hook::Set<DWORD>(0x403E8C, WEAPON_LIMIT);
		Utils::Hook::Set<DWORD>(0x41BC34, WEAPON_LIMIT);
		Utils::Hook::Set<DWORD>(0x42EB42, WEAPON_LIMIT);
		Utils::Hook::Set<DWORD>(0x44FA7B, WEAPON_LIMIT);
		Utils::Hook::Set<DWORD>(0x474E0D, WEAPON_LIMIT);
		Utils::Hook::Set<DWORD>(0x48E8F2, WEAPON_LIMIT);
		Utils::Hook::Set<DWORD>(0x492647, WEAPON_LIMIT);
		Utils::Hook::Set<DWORD>(0x494585, WEAPON_LIMIT);
		Utils::Hook::Set<DWORD>(0x4945DB, WEAPON_LIMIT);
		Utils::Hook::Set<DWORD>(0x4B1F96, WEAPON_LIMIT);
		Utils::Hook::Set<DWORD>(0x4D4A99, WEAPON_LIMIT);
		Utils::Hook::Set<DWORD>(0x4DD566, WEAPON_LIMIT);
		Utils::Hook::Set<DWORD>(0x4E3683, WEAPON_LIMIT); // Configstring
		Utils::Hook::Set<DWORD>(0x58609F, WEAPON_LIMIT);
		Utils::Hook::Set<DWORD>(0x586CAE, WEAPON_LIMIT);
		Utils::Hook::Set<DWORD>(0x58F7BE, WEAPON_LIMIT);
		Utils::Hook::Set<DWORD>(0x58F7D9, WEAPON_LIMIT);
		Utils::Hook::Set<DWORD>(0x58F82D, WEAPON_LIMIT);
		Utils::Hook::Set<DWORD>(0x5D6C8B, WEAPON_LIMIT);
		Utils::Hook::Set<DWORD>(0x5D6CF7, WEAPON_LIMIT);
		Utils::Hook::Set<DWORD>(0x5E24D5, WEAPON_LIMIT);
		Utils::Hook::Set<DWORD>(0x5E2604, WEAPON_LIMIT);
		Utils::Hook::Set<DWORD>(0x5E2828, WEAPON_LIMIT);
		Utils::Hook::Set<DWORD>(0x5E2B4F, WEAPON_LIMIT);
		Utils::Hook::Set<DWORD>(0x5E366C, WEAPON_LIMIT);
		Utils::Hook::Set<DWORD>(0x5F2614, WEAPON_LIMIT);
		Utils::Hook::Set<DWORD>(0x5F7187, WEAPON_LIMIT);
		Utils::Hook::Set<DWORD>(0x5FECF9, WEAPON_LIMIT);

		Utils::Hook::Set<int>(0x586CC3, -(WEAPON_LIMIT));

		// Reference: https://courses.engr.illinois.edu/ece390/books/artofasm/CH09/CH09-6.html (See 9.5.2 Division Without DIV and IDIV)
		// And http://reverseengineering.stackexchange.com/questions/1397/how-can-i-reverse-optimized-integer-division-modulo-by-constant-operations
		// The game's magic number is computed using this formula: (1 / 1200) * (2 ^ (32 + 7)
		// I'm too lazy to generate the new magic number, so we can make use of the fact that using powers of 2 as scales allows to change the compensating shift
		static_assert(((WEAPON_LIMIT / BASEGAME_WEAPON_LIMIT) * BASEGAME_WEAPON_LIMIT) == WEAPON_LIMIT && (WEAPON_LIMIT / BASEGAME_WEAPON_LIMIT) != 0 && !((WEAPON_LIMIT / BASEGAME_WEAPON_LIMIT) & ((WEAPON_LIMIT / BASEGAME_WEAPON_LIMIT) - 1)), "WEAPON_LIMIT / 1200 is not a power of 2!");
		const unsigned char compensation = 7 + static_cast<unsigned char>(log2(WEAPON_LIMIT / BASEGAME_WEAPON_LIMIT)); // 7 is the compensation the game uses
		Utils::Hook::Set<BYTE>(0x49263D, compensation);
		Utils::Hook::Set<BYTE>(0x5E250C, compensation);
		Utils::Hook::Set<BYTE>(0x5E2B43, compensation);
		Utils::Hook::Set<BYTE>(0x5E352F, compensation);
		Utils::Hook::Set<BYTE>(0x5FECEF, compensation);

		static int bg_weaponCompleteDefs[WEAPON_LIMIT];
		Utils::Hook::Set<DWORD>(0x4B35E1, sizeof(bg_weaponCompleteDefs));
		Utils::Hook::Set(0x44CE07, bg_weaponCompleteDefs);
		Utils::Hook::Set(0x45C671, bg_weaponCompleteDefs);
		Utils::Hook::Set(0x45C6A4, bg_weaponCompleteDefs);
		Utils::Hook::Set(0x4B35E8, bg_weaponCompleteDefs);
		Utils::Hook::Set(0x4B3643, bg_weaponCompleteDefs);
		Utils::Hook::Set(0x4CE903, bg_weaponCompleteDefs);
		Utils::Hook::Set(0x4CE927, bg_weaponCompleteDefs);
		Utils::Hook::Set(0x4E6EC7, bg_weaponCompleteDefs);
		Utils::Hook::Set(0x57B69A, bg_weaponCompleteDefs);
		Utils::Hook::Set(0x57B910, bg_weaponCompleteDefs);
		Utils::Hook::Set(0x57B925, bg_weaponCompleteDefs);
		Utils::Hook::Set(0x57BA22, bg_weaponCompleteDefs);
		Utils::Hook::Set(0x57BA51, bg_weaponCompleteDefs);
		Utils::Hook::Set(0x57BA78, bg_weaponCompleteDefs);

		static int bg_weaponDefs[WEAPON_LIMIT];
		Utils::Hook::Set<DWORD>(0x4B35F2, sizeof(bg_weaponDefs));
		Utils::Hook::Set(0x42F697, bg_weaponDefs);
		Utils::Hook::Set(0x440EB7, bg_weaponDefs);
		Utils::Hook::Set(0x45C67D, bg_weaponDefs);
		Utils::Hook::Set(0x45C685, bg_weaponDefs);
		Utils::Hook::Set(0x45C6B0, bg_weaponDefs);
		Utils::Hook::Set(0x4B35F9, bg_weaponDefs);
		Utils::Hook::Set(0x4B364B, bg_weaponDefs);
		Utils::Hook::Set(0x57B693, bg_weaponDefs);
		Utils::Hook::Set(0x57B75D, bg_weaponDefs);
		Utils::Hook::Set(0x57B809, bg_weaponDefs);
		Utils::Hook::Set(0x57B8D3, bg_weaponDefs);
		Utils::Hook::Set(0x57B96D, bg_weaponDefs);
		Utils::Hook::Set(0x57BA32, bg_weaponDefs);

		static int bg_weapClips[WEAPON_LIMIT];
		Utils::Hook::Set<DWORD>(0x4B3603, sizeof(bg_weapClips));
		Utils::Hook::Set(0x4B360A, bg_weapClips);
		Utils::Hook::Set(0x4B3666, bg_weapClips);
		Utils::Hook::Set(0x57B993, bg_weapClips);
		Utils::Hook::Set(0x57B9E1, bg_weapClips);
		Utils::Hook::Set(0x57B9EA, bg_weapClips);

		static int bg_sharedAmmoCaps[WEAPON_LIMIT];
		Utils::Hook::Set<DWORD>(0x4B3614, sizeof(bg_sharedAmmoCaps));
		Utils::Hook::Set(0x414D27, bg_sharedAmmoCaps);
		Utils::Hook::Set(0x4B361B, bg_sharedAmmoCaps);
		Utils::Hook::Set(0x4B365B, bg_sharedAmmoCaps);
		Utils::Hook::Set(0x57B83B, bg_sharedAmmoCaps);
		Utils::Hook::Set(0x57B870, bg_sharedAmmoCaps);
		Utils::Hook::Set(0x57B87D, bg_sharedAmmoCaps);
		Utils::Hook::Set(0x57B89C, bg_sharedAmmoCaps);
		Utils::Hook::Set(0x57B8E0, bg_sharedAmmoCaps);
		Utils::Hook::Set(0x57B901, bg_sharedAmmoCaps);

		static int bg_weapAmmoTypes[WEAPON_LIMIT];
		Utils::Hook::Set<DWORD>(0x4B3625, sizeof(bg_weapAmmoTypes));
		Utils::Hook::Set(0x4B362C, bg_weapAmmoTypes);
		Utils::Hook::Set(0x4B3650, bg_weapAmmoTypes);
		Utils::Hook::Set(0x57B783, bg_weapAmmoTypes);
		Utils::Hook::Set(0x57B7D1, bg_weapAmmoTypes);
		Utils::Hook::Set(0x57B7DA, bg_weapAmmoTypes);

		static int weaponStrings[WEAPON_LIMIT * 2]; // string + hash
		Utils::Hook::Set<DWORD>(0x504E01, sizeof(weaponStrings));
		Utils::Hook::Set<DWORD>(0x4C77DC, sizeof(weaponStrings));
		Utils::Hook::Set(0x4B72DC, weaponStrings);
		Utils::Hook::Set(0x4C77E2, weaponStrings);
		Utils::Hook::Set(0x504E08, weaponStrings);
		Utils::Hook::Set(0x795584, weaponStrings);
		Utils::Hook::Set(0x7955FC, weaponStrings);
		Utils::Hook::Set(0x4B72E8, &weaponStrings[1]);

		static char cg_weaponsArray[32 * WEAPON_LIMIT];
		Utils::Hook::Set<DWORD>(0x4E3300, sizeof(cg_weaponsArray));
		Utils::Hook::Set<DWORD>(0x4E3305, sizeof(cg_weaponsArray));
		Utils::Hook::Set<DWORD>(0x4F1927, sizeof(cg_weaponsArray));
		Utils::Hook::Set<DWORD>(0x4F192C, sizeof(cg_weaponsArray));
		Utils::Hook::Set(0x4172D8, cg_weaponsArray);
		Utils::Hook::Set(0x45C96F, cg_weaponsArray);
		Utils::Hook::Set(0x45C9C7, cg_weaponsArray);
		Utils::Hook::Set(0x47F417, cg_weaponsArray);
		Utils::Hook::Set(0x4BA571, cg_weaponsArray);
		Utils::Hook::Set(0x4E330B, cg_weaponsArray);
		Utils::Hook::Set(0x4EF56C, cg_weaponsArray);
		Utils::Hook::Set(0x4F1934, cg_weaponsArray);
		Utils::Hook::Set(0x58B879, cg_weaponsArray);
		Utils::Hook::Set(0x58B98A, cg_weaponsArray);
		Utils::Hook::Set(0x58CCF7, cg_weaponsArray);
		Utils::Hook::Set(0x59BF05, cg_weaponsArray);
		Utils::Hook::Set(0x59C39B, cg_weaponsArray);
		Utils::Hook::Set(0x446D30, &cg_weaponsArray[8]);
		Utils::Hook::Set(0x59BD68, &cg_weaponsArray[13]);
		Utils::Hook::Set(0x58D0AE, &cg_weaponsArray[20]);

		static int cg_weaponsStaticArray[3 * WEAPON_LIMIT];
		Utils::Hook::Set<DWORD>(0x4E3322, sizeof(cg_weaponsStaticArray));
		Utils::Hook::Set<DWORD>(0x4F1912, sizeof(cg_weaponsStaticArray));
		Utils::Hook::Set(0x4548DE, cg_weaponsStaticArray);
		Utils::Hook::Set(0x4E3328, cg_weaponsStaticArray);
		Utils::Hook::Set(0x4EF57A, cg_weaponsStaticArray);
		Utils::Hook::Set(0x4F1919, cg_weaponsStaticArray);
		Utils::Hook::Set(0x59C095, cg_weaponsStaticArray);
		Utils::Hook::Set(0x59C09D, cg_weaponsStaticArray);

		PatchHintStrings();

		// Patch bg_weaponDefs on the stack
		Utils::Hook::Set<DWORD>(0x40C31D, sizeof(bg_weaponDefs));
		Utils::Hook::Set<DWORD>(0x40C32F, sizeof(bg_weaponDefs));
		Utils::Hook::Set<DWORD>(0x40C311, 0x258C + ((sizeof(bg_weaponDefs) * 2) - (BASEGAME_WEAPON_LIMIT * 4 * 2)));
		Utils::Hook::Set<DWORD>(0x40C45F, 0x258C + ((sizeof(bg_weaponDefs) * 2) - (BASEGAME_WEAPON_LIMIT * 4 * 2)));
		Utils::Hook::Set<DWORD>(0x40C478, 0x258C + ((sizeof(bg_weaponDefs) * 2) - (BASEGAME_WEAPON_LIMIT * 4 * 2)));
		Utils::Hook::Set<DWORD>(0x40C434, 0x258C + ((sizeof(bg_weaponDefs) * 2) - (BASEGAME_WEAPON_LIMIT * 4 * 2)));
		Utils::Hook::Set<DWORD>(0x40C434, 0x258C + ((sizeof(bg_weaponDefs) * 2) - (BASEGAME_WEAPON_LIMIT * 4 * 2)));
		// Move second buffer pointers
		Utils::Hook::Set<DWORD>(0x40C336, 0x12E4 + ((sizeof(bg_weaponDefs)) - (BASEGAME_WEAPON_LIMIT * 4)));
		Utils::Hook::Set<DWORD>(0x40C3C6, 0x12DC + ((sizeof(bg_weaponDefs)) - (BASEGAME_WEAPON_LIMIT * 4)));
		Utils::Hook::Set<DWORD>(0x40C3CE, 0x12DC + ((sizeof(bg_weaponDefs)) - (BASEGAME_WEAPON_LIMIT * 4)));
		// Move arg0 pointers
		Utils::Hook::Set<DWORD>(0x40C365, 0x259C + ((sizeof(bg_weaponDefs) * 2) - (BASEGAME_WEAPON_LIMIT * 4 * 2)));
		Utils::Hook::Set<DWORD>(0x40C44E, 0x259C + ((sizeof(bg_weaponDefs) * 2) - (BASEGAME_WEAPON_LIMIT * 4 * 2)));
		Utils::Hook::Set<DWORD>(0x40C467, 0x259C + ((sizeof(bg_weaponDefs) * 2) - (BASEGAME_WEAPON_LIMIT * 4 * 2)));
		// Move arg4 pointers
		Utils::Hook::Set<DWORD>(0x40C344, 0x25B4 + ((sizeof(bg_weaponDefs) * 2) - (BASEGAME_WEAPON_LIMIT * 4 * 2)));

		// Patch bg_sharedAmmoCaps on the stack
		Utils::Hook::Set<DWORD>(0x4F76E6, sizeof(bg_sharedAmmoCaps));
		Utils::Hook::Set<DWORD>(0x4F7621, 0x12C8 + (sizeof(bg_sharedAmmoCaps) - (BASEGAME_WEAPON_LIMIT * 4)));
		Utils::Hook::Set<DWORD>(0x4F76AF, 0x12C8 + (sizeof(bg_sharedAmmoCaps) - (BASEGAME_WEAPON_LIMIT * 4)));
		Utils::Hook::Set<DWORD>(0x4F76DA, 0x12C8 + (sizeof(bg_sharedAmmoCaps) - (BASEGAME_WEAPON_LIMIT * 4)));
		Utils::Hook::Set<DWORD>(0x4F77C5, 0x12C8 + (sizeof(bg_sharedAmmoCaps) - (BASEGAME_WEAPON_LIMIT * 4)));
		// Move arg0 pointers
		Utils::Hook::Set<DWORD>(0x4F766D, 0x12DC + (sizeof(bg_sharedAmmoCaps) - (BASEGAME_WEAPON_LIMIT * 4)));
		Utils::Hook::Set<DWORD>(0x4F76B7, 0x12DC + (sizeof(bg_sharedAmmoCaps) - (BASEGAME_WEAPON_LIMIT * 4)));
		Utils::Hook::Set<DWORD>(0x4F76FB, 0x12EC + (sizeof(bg_sharedAmmoCaps) - (BASEGAME_WEAPON_LIMIT * 4)));
		// Move arg4 pointers
		Utils::Hook::Set<DWORD>(0x4F7630, 0x12DC + (sizeof(bg_sharedAmmoCaps) - (BASEGAME_WEAPON_LIMIT * 4)));
	}

	void* Weapon::LoadNoneWeaponHook()
	{
		// load anim scripts now, rather than a bit later on
		Utils::Hook::Call<void()>(0x4E46A0)();

		return Game::DB_FindXAssetHeader(Game::XAssetType::ASSET_TYPE_WEAPON, "none").data;
	}

	void __declspec(naked) Weapon::LoadNoneWeaponHookStub()
	{
		__asm
		{
			jmp LoadNoneWeaponHook
		}
	}

	void __declspec(naked) Weapon::CG_UpdatePrimaryForAltModeWeapon_Stub()
	{
		__asm
		{
			mov eax, 0x440EB0 // BG_GetWeaponDef
			call eax
			add esp, 0x4

			test eax, eax
			jz null

			// Game code
			push 0x59E349
			retn

			null :
			mov al, 1
				ret
		}
	}

	void __declspec(naked) Weapon::CG_SelectWeaponIndex_Stub()
	{
		__asm
		{
			mov eax, 0x440EB0 // BG_GetWeaponDef
			call eax
			add esp, 0x4

			test eax, eax
			jz null

			// Game code
			push 0x48BB2D
			ret

			null :
			push 0x48BB1F // Exit function
				ret
		}
	}

	void __declspec(naked) Weapon::WeaponEntCanBeGrabbed_Stub()
	{
		using namespace Game;

		__asm
		{
			cmp dword ptr[esp + 0x8], 0x0
			jz touched

			push 0x56E82C
			ret

			touched :
			test dword ptr[edi + 0x2BC], PWF_DISABLE_WEAPON_PICKUP
				jnz exit_func

				// Game code
				test eax, eax
				jz continue_func

				exit_func :
			xor eax, eax
				ret

				continue_func :
			push 0x56E84C
				ret
		}
	}

	__declspec(naked) void Weapon::JavelinResetHook_Stub()
	{
		static DWORD PM_Weapon_OffHandEnd_t = 0x577A10;

		__asm
		{
			call PM_Weapon_OffHandEnd_t

			push eax
			mov eax, BGWeaponOffHandFix
			cmp byte ptr[eax + 0x10], 1
			pop eax

			jne safeReturn

			mov dword ptr[esi + 0x34], 0 // playerState_s.grenadeTimeLeft

			safeReturn:
			pop edi
				pop esi
				pop ebx
				ret
		}
	}

	void Weapon::BG_WeaponFireRecoil_Stub(
		void* ps,
		float* recoilSpeed,
		float* kickAVel,
		unsigned int* holdrand,
		Game::PlayerHandIndex hand
	)
	{
		float adjustedRecoilSpeed[3]{};
		float adjustedKick[3]{};


		Utils::Hook::Call<void(void*, float*, float*, unsigned int*, Game::PlayerHandIndex)>(0x4A5FE0)(
			ps,
			adjustedRecoilSpeed,
			adjustedKick,
			holdrand,
			hand
		);

		for (size_t axis = 0; axis < 3; axis++)
		{
			recoilSpeed [axis] = adjustedRecoilSpeed[axis] * CGRecoilMultiplier->current.value;
			kickAVel [axis] = adjustedKick[axis] * CGRecoilMultiplier->current.value;
		}
	}

	void Weapon::PlayerCmd_InitialWeaponRaise(const Game::scr_entref_t entref)
	{
		auto* ent = GSC::Script::Scr_GetPlayerEntity(entref);
		const auto* weapon = Game::Scr_GetString(0);
		const auto index = Game::G_GetWeaponIndexForName(weapon);

		auto* ps = &ent->client->ps;
		if (!Game::BG_IsWeaponValid(ps, index))
		{
			Game::Scr_Error(Utils::String::VA("invalid InitialWeaponRaise: %s", weapon));
			return;
		}

		assert(ps);

		if (!index)
		{
			return;
		}

		auto* equippedWeapon = Game::BG_GetEquippedWeaponState(ps, index);
		if (!equippedWeapon)
		{
			return;
		}

		equippedWeapon->usedBefore = false;
		Game::Player_SwitchToWeapon(ent);
	}

	void Weapon::PlayerCmd_FreezeControlsAllowLook(const Game::scr_entref_t entref)
	{
		const auto* ent = GSC::Script::Scr_GetPlayerEntity(entref);

		if (Game::Scr_GetInt(0))
		{
			ent->client->ps.weapCommon.weapFlags |= Game::PWF_DISABLE_WEAPONS;
			ent->client->flags |= Game::CF_BIT_DISABLE_USABILITY;
		}
		else
		{
			ent->client->ps.weapCommon.weapFlags &= ~Game::PWF_DISABLE_WEAPONS;
			ent->client->flags &= ~Game::CF_BIT_DISABLE_USABILITY;
		}
	}

	void Weapon::AddScriptMethods()
	{
		GSC::Script::AddMethod("DisableWeaponPickup", [](const Game::scr_entref_t entref)
			{
				const auto* ent = GSC::Script::Scr_GetPlayerEntity(entref);

				ent->client->ps.weapCommon.weapFlags |= Game::PWF_DISABLE_WEAPON_PICKUP;
			});

		GSC::Script::AddMethod("EnableWeaponPickup", [](const Game::scr_entref_t entref)
			{
				const auto* ent = GSC::Script::Scr_GetPlayerEntity(entref);

				ent->client->ps.weapCommon.weapFlags &= ~Game::PWF_DISABLE_WEAPON_PICKUP;
			});

		// PlayerCmd_AreControlsFrozen GSC function from Black Ops 2
		GSC::Script::AddMethod("AreControlsFrozen", [](Game::scr_entref_t entref) // Usage: self AreControlsFrozen();
			{
				const auto* ent = GSC::Script::Scr_GetPlayerEntity(entref);
				Game::Scr_AddBool((ent->client->flags & Game::CF_BIT_FROZEN) != 0);
			});

		GSC::Script::AddMethod("InitialWeaponRaise", PlayerCmd_InitialWeaponRaise);
		GSC::Script::AddMethod("FreezeControlsAllowLook", PlayerCmd_FreezeControlsAllowLook);
	}

	Weapon::Weapon()
	{
		PatchLimit();

		// BG_LoadWEaponCompleteDef_FastFile
		Utils::Hook(0x57B650, LoadWeaponCompleteDef, HOOK_JUMP).install()->quick();
		// Disable warning if raw weapon file cannot be found
		Utils::Hook::Nop(0x57AF60, 5);

		// weapon asset existence check
		Utils::Hook::Nop(0x408228, 5); // find asset header
		Utils::Hook::Nop(0x408230, 5); // is asset default
		Utils::Hook::Nop(0x40823A, 2); // jump

		// Automatically register weapons, even if the level is not loading
		Utils::Hook::Nop(0x49E547, 2);
		Utils::Hook::Set<BYTE>(0x44F240, 0xEB);

		// Skip double loading for fs_game
		Utils::Hook::Set<BYTE>(0x4081FD, 0xEB);

		// Weapon swap fix
		Utils::Hook::Nop(0x4B3670, 5);
		Utils::Hook(0x57B4F0, LoadNoneWeaponHookStub, HOOK_JUMP).install()->quick();

		// Clear weapons independently from fs_game
		Utils::Hook::Nop(0x452C1D, 2);
		Utils::Hook::Nop(0x452C24, 5);

		Utils::Hook(0x59E341, CG_UpdatePrimaryForAltModeWeapon_Stub, HOOK_JUMP).install()->quick();
		Utils::Hook(0x48BB25, CG_SelectWeaponIndex_Stub, HOOK_JUMP).install()->quick();

		AssertOffset(Game::playerState_s, Game::playerState_s::weapCommon.weapFlags, 0x2BC);
		Utils::Hook(0x56E825, WeaponEntCanBeGrabbed_Stub, HOOK_JUMP).install()->quick();

		// Javelin fix (PM_Weapon_OffHandEnd)
		AssertOffset(Game::playerState_s, grenadeTimeLeft, 0x34);
		BGWeaponOffHandFix = Game::Dvar_RegisterBool("bg_weaponOffHandFix", true, Game::DVAR_CODINFO, "Reset grenadeTimeLeft after using off hand weapon");
		Utils::Hook(0x578F52, JavelinResetHook_Stub, HOOK_JUMP).install()->quick();

		CGRecoilMultiplier = Game::Dvar_RegisterFloat("cg_recoilMultiplier",
			1.0f, 0.0f, 1000.0f, Game::DVAR_CHEAT,
			"The scale applied to the player recoil when firing");
		Utils::Hook(0x44D90B, BG_WeaponFireRecoil_Stub, HOOK_CALL).install()->quick();
		Utils::Hook(0x4FB2D7, BG_WeaponFireRecoil_Stub, HOOK_CALL).install()->quick();

		AddScriptMethods();
	}
}
