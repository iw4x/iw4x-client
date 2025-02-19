#include <STDInclude.hpp>

#include "rapidjson/document.h"

#include "GSC/Script.hpp"

#include "PlayerSkins.hpp"
#include "Events.hpp"

namespace Components
{
	constexpr auto USERINFO_KEY = "skin";
	constexpr auto MAX_ALLOWED_BONES = 140;

	const std::string PlayerSkins::heads[] = {
		"",
		"head_airport_a",
		"head_airport_b",
		"head_airport_d",
		"head_opforce_fsb_a",
		"head_opforce_fsb_b",
		"head_secret_service_a",
		"head_secret_service_b",
		"head_secret_service_c",
		"head_urban_civ_female_a",
		"head_urban_civ_female_b",
		"head_urban_civ_male_a",
		"head_urban_civ_male_b",
		"head_urban_civ_male_c",
		"head_urban_civ_male_d",
		"head_opforce_merc_b_hat",
		"head_hero_ghost_forest",
		"head_shadow_co_b",
		"head_shadow_co_c",
		"head_opforce_merc_c",
		"head_opforce_merc_d",
		"head_opforce_merc_e",
		"head_opforce_merc_f",
		"head_gign_a",
		"head_gign_b",
		"head_gign_c",
		"head_gign_d",
		"head_gign_generic_gasmask",
		"head_gign_saber_gasmask",
		"head_hero_grinch_gasmask_delta",
		"head_hero_sandman_gasmask",
		"head_london_female_a",
		"head_london_female_b",
		"head_london_male_a",
		"head_london_male_b",
		"head_india_female_a",
		"head_india_female_b",
		"head_tank_a",
		"head_tank_b",
		"head_tank_bb",
		"head_tank_c",
		"head_russian_military_aa",
		"head_russian_military_bb",
		"head_russian_military_cc",
		"head_russian_military_a",
		"head_russian_military_c",
		"head_price_europe_b_winter",
		"head_price_europe_c_nvg",
		"head_russian_naval_a",
		"head_russian_naval_b",
		"head_russian_naval_c",
		"head_so_juggernaut_blue_hat",
		"head_fso_a",
		"head_hero_yuri_a",
		"head_hero_sandman_delta",
		"russian_presidents_daughter_head",
		"russian_presidents_daughter_head_dirty",
		"head_chemwar_russian_a",
		"head_chemwar_russian_d",
		"head_chemwar_russian_e",
		"head_london_cop_a",
		"head_london_cop_b",
		"head_doctor",
		"head_opforce_russian_urban_sniper",
		"head_opforce_russian_air_sniper",
		"head_russianwoodland_a_dusty",
		"head_russianwoodland_b",
		"head_russianwoodland_c"
	};

	const std::string PlayerSkins::bodies[] = {
		"",
		"body_secret_service_assault_a",
		"body_secret_service_shotgun",
		"body_secret_service_smg",
		"body_urban_civ_female_a",
		"body_urban_civ_female_b",
		"body_urban_civ_male_aa",
		"body_urban_civ_male_ab",
		"body_urban_civ_male_ac",
		"body_urban_civ_male_ba",
		"body_urban_civ_male_bb",
		"body_urban_civ_male_bc",
		"body_airport_com_a",
		"body_airport_com_b",
		"body_airport_com_c",
		"body_airport_com_d",
		"body_city_civ_male_a",
		"body_opforce_fsb_assault_a",
		"body_opforce_fsb_shotgun",
		"body_opforce_fsb_smg",
		"body_hero_sandman_delta",
		"body_london_female_a",
		"body_london_female_aa",
		"body_london_female_aaa",
		"body_london_female_b",
		"body_london_female_bb",
		"body_london_female_bbb",
		"body_london_female_c",
		"body_london_female_cc",
		"body_london_female_ccc",
		"body_london_male_a",
		"body_london_male_b",
		"body_london_male_c",
		"body_dubai_male_a",
		"body_dubai_male_a_alt",
		"body_dubai_male_b",
		"body_dubai_male_b_alt",
		"body_dubai_male_c",
		"body_dubai_male_c_alt",
		"body_india_male_a",
		"body_india_male_b",
		"body_india_male_a_alt",
		"body_india_male_b_alt",
		"body_india_female_a",
		"body_india_female_b",
		"body_slum_civ_female_ba",
		"body_prague_civ_male_a",
		"body_prague_civ_male_b",
		"body_prague_civ_male_c",
		"body_prague_civ_male_d",
		"body_prague_civ_male_e",
		"body_prague_civ_male_f",
		"body_prague_civ_male_aa",
		"body_prague_civ_male_bb",
		"body_prague_civ_male_cc",
		"body_prague_civ_male_dd",
		"body_prague_civ_male_ff",
		"body_prague_civ_male_aaa",
		"body_prague_civ_male_bbb",
		"body_prague_civ_male_ccc",
		"body_prague_civ_male_ddd",
		"body_prague_civ_male_eee",
		"body_prague_civ_male_fff",
		"body_russian_president",
		"body_forest_tf141_ghost",
		"", // available
		"body_chemwar_russian_assault_cc",
		"body_chemwar_russian_assault_c",
		"body_russian_naval_assault_ff",
		"body_juggernaut_nohelmet",
		"body_doctor"
	};

	Game::scr_string_t PlayerSkins::headsScriptStrings[ARRAYSIZE(PlayerSkins::heads)]{};
	Game::scr_string_t PlayerSkins::bodiesScriptStrings[ARRAYSIZE(PlayerSkins::bodies)]{};

	Dvar::Var PlayerSkins::localHeadIndexDvar;
	Dvar::Var PlayerSkins::localBodyIndexDvar;
	Dvar::Var PlayerSkins::localEnableHeadDvar;
	Dvar::Var PlayerSkins::localEnableBodyDvar;
	Dvar::Var PlayerSkins::skinTryOut;
	Dvar::Var PlayerSkins::sv_allowSkins;
	Dvar::Var PlayerSkins::sv_overrideTeamSkins;

	std::unordered_set<int> PlayerSkins::forbiddenHeadBodyCombinations;

	Skin PlayerSkins::currentSkin;

	// We hook cdecl so no need to write assembly code
	uint32_t PlayerSkins::GetTrueSkillForGametype([[maybe_unused]] int localClientIndex, [[maybe_unused]] char* gametype)
	{
		// localClientIndex will always be zero
		// gametype will always be TDM

		// We can return anything here and it will be replicated to the party! Woohoo!

		RefreshPlayerSkinFromDvars();

		return currentSkin.intValue;
	}

	void PlayerSkins::RegisterConstantStrings()
	{
		for (size_t i = 0; i < ARRAYSIZE(heads); i++)
		{
			headsScriptStrings[i] =  static_cast<Game::scr_string_t>(Game::SL_GetString(heads[i].data(), 0));

			Components::Logger::Print("head #{} registered to string index [{}] ({})\n", i, headsScriptStrings[i], heads[i]);
		}

		for (size_t i = 0; i < ARRAYSIZE(bodies); i++)
		{
			bodiesScriptStrings[i] = static_cast<Game::scr_string_t>(Game::SL_GetString(bodies[i].data(), 0));

			Components::Logger::Print("body #{} registered to string index [{}] ({})\n", i, bodiesScriptStrings[i], bodies[i]);
		}
	}

	void PlayerSkins::CheckForbiddenHeadBodyCombinations()
	{
		forbiddenHeadBodyCombinations.clear();

		uint16_t headBonesCount[ARRAYSIZE(heads)]{};
		uint16_t bodyBonesCount[ARRAYSIZE(bodies)]{};

		for (size_t i = 0; i < ARRAYSIZE(heads); i++)
		{
			if (!heads[i].empty())
			{
				const auto entry = Game::DB_FindXAssetEntry(Game::XAssetType::ASSET_TYPE_XMODEL, heads[i].data());
				if (entry && entry->asset.header.data)
				{
					headBonesCount[i] = entry->asset.header.model->numBones;
				}
				else
				{
					Logger::Error(Game::ERR_SCRIPT, "Head {} is nowhere to be found!\n", heads[i]);
				}
			}
		}

		for (size_t i = 0; i < ARRAYSIZE(bodies); i++)
		{
			if (!bodies[i].empty())
			{
				const auto entry = Game::DB_FindXAssetEntry(Game::XAssetType::ASSET_TYPE_XMODEL, bodies[i].data());
				if (entry && entry->asset.header.data)
				{
					bodyBonesCount[i] = entry->asset.header.model->numBones;
				}
				else
				{
					Logger::Error(Game::ERR_SCRIPT, "Body {} is nowhere to be found!\n", bodies[i]);
				}
			}
		}

		// Trying every combination
		for (size_t bodyIndex = 0; bodyIndex < ARRAYSIZE(bodies); bodyIndex++)
		{
			const auto bodyBoneCount = bodyBonesCount[bodyIndex];

			for (size_t headIndex = 0; headIndex < ARRAYSIZE(heads); headIndex++)
			{
				const auto headBoneCount = headBonesCount[headIndex];

				if (headBoneCount + bodyBoneCount >= MAX_ALLOWED_BONES)
				{
					if ((*Game::com_developer)->current.value)
					{
						Logger::Print(
							"Notice: Skin {} + {} will not be allowed ({} + {}= {} bones total)\n",
							heads[headIndex],
							bodies[bodyIndex],
							headBoneCount,
							bodyBoneCount,
							headBoneCount + bodyBoneCount
						);
					}

					forbiddenHeadBodyCombinations.emplace((headIndex << 8) | bodyIndex);
				}
				else
				{
					if ((*Game::com_developer)->current.value)
					{
						Logger::Print(
							"Notice: Skin {} + {} is allowed ({} + {}= {} bones total)\n",
							heads[headIndex],
							bodies[bodyIndex],
							headBoneCount,
							bodyBoneCount,
							headBoneCount + bodyBoneCount
						);
					}
				}
			}
		}
	}

	void PlayerSkins::RefreshPlayerSkinFromDvars()
	{
		currentSkin.enableBody = localEnableBodyDvar.get<bool>() ? 1 : 0;
		currentSkin.enableHead = localEnableHeadDvar.get<bool>() ? 1 : 0;
		currentSkin.bodyIndex = localBodyIndexDvar.get<int>();
		currentSkin.headIndex = localHeadIndexDvar.get<int>();

		SanitizeSkin(currentSkin);
	}

	void PlayerSkins::SanitizeSkin(Skin& skin)
	{
		// Clamp to the realm of possible
		skin.headIndex = std::clamp(skin.headIndex, (unsigned)0, ARRAYSIZE(heads) - 1);
		skin.bodyIndex = std::clamp(skin.bodyIndex, (unsigned)0, ARRAYSIZE(bodies) - 1);
	}

	void PlayerSkins::Info_SetValueForKey(const char* infoString, const char* key, const char* data)
	{
		Utils::Hook::Call<void(const char*, const char*, const char*)>(0x4AE560)(infoString, key, data); // Info_SetValueForKey original

		RefreshPlayerSkinFromDvars();

		const auto skinStr = std::to_string(currentSkin.intValue);
		Utils::Hook::Call<void(const char*, const char*, const char*)>(0x4AE560)(infoString, USERINFO_KEY, skinStr.c_str());
	}

	bool PlayerSkins::HasAuthorizedBoneCount(const Skin& skin, std::string& err)
	{
		Game::XModel* body = nullptr;
		Game::XModel* head = nullptr;

		const auto bodyName = GetBodyName(skin);

		if (bodyName)
		{
			const auto str = Game::SL_ConvertToString(bodyName);

			if (str)
			{
				const auto bodyAsset = Game::DB_FindXAssetHeader(Game::ASSET_TYPE_XMODEL, Game::SL_ConvertToString(bodyName));

				if (bodyAsset.data == nullptr)
				{
					err = std::format("Missing body '{}' ??", bodyName);
					return false;
				}
				else
				{
					body = bodyAsset.model;
				}
			}
			else
			{
				err = std::format("NULL string ref on skin body '{}' !!", skin.bodyIndex);
				return false;
			}
		}
		else
		{
			// No body set
			return true;
		}

		const auto headName = GetHeadName(skin);

		if (headName)
		{
			const auto str = Game::SL_ConvertToString(headName);

			if (str)
			{
				const auto headAsset = Game::DB_FindXAssetHeader(Game::ASSET_TYPE_XMODEL, str);

				if (headAsset.data == nullptr)
				{
					err = std::format("Missing head '{}' ??", headName);
					return false;
				}
				else
				{
					head = headAsset.model;
				}
			}
			else
			{
				err = std::format("NULL string ref on skin head '{}' !!", skin.headIndex);
				return false;
			}
		}
		else
		{
			// No head set
			return true;
		}

		const auto totalBoneCount = body->numBones + head->numBones;

		if (totalBoneCount < MAX_ALLOWED_BONES) // 192 Is max bone count
		{
			return true;
		}
		else
		{
			err = std::format("Could not apply skin head {} - too many bones! ({}). {} ({}) + {} ({}) > {}", headName, totalBoneCount, bodyName, body->numBones, headName, head->numBones, MAX_ALLOWED_BONES);

			return false;
		}
	}

	Game::scr_string_t PlayerSkins::GetHeadName(const Skin& skin)
	{
		return skin.enableHead ? headsScriptStrings[skin.headIndex] : headsScriptStrings[0];
	}

	Game::scr_string_t PlayerSkins::GetBodyName(const Skin& skin)
	{
		return skin.enableBody ? bodiesScriptStrings[skin.bodyIndex] : bodiesScriptStrings[0];
	}

	static int highest = 0;
	static bool initialized = false;
	struct MemoryNode
	{
	  unsigned int padding[3];
	};

	constexpr auto MT_NODE_SIZE = 12;
	
	struct __declspec(align(128)) scrMemTreeGlob_t
	{
	  MemoryNode nodes[65536];
	  unsigned int nodeBits[2048];
	  unsigned int sizeBits[5];
	  unsigned int head[17];
	  unsigned int backtrackAmount[17];
	  int totalAlloc;
	  int totalAllocBuckets;
	};
	
	struct $119B815E6C15BED54461C272BD343858
	{
	  unsigned __int32 refCount : 16;
	  unsigned __int32 user : 8;
	  unsigned __int32 byteLen : 8;
	};

	union $156C516D3E6908D9990BD5CCD794911D
	{
	  $119B815E6C15BED54461C272BD343858 __s0;
	  volatile int data;
	};

	struct __declspec(align(4)) RefString
	{
	  $156C516D3E6908D9990BD5CCD794911D ___u0;
	  char str[1];
	};


	const char * PlayerSkins::SL_ConvertToString(int a1)
	{
		Game::scrMemTreePub_t* tree = reinterpret_cast<Game::scrMemTreePub_t*>(0x1DC2200);

		RefString* buff = reinterpret_cast<RefString*>(&tree->mt_buffer[a1 * MT_NODE_SIZE]);

		const char * str = Game::SL_ConvertToString(static_cast<Game::scr_string_t>(a1));

		assert(buff->str == str);

		if(initialized){
			for (size_t i = 0; i < ARRAYSIZE(PlayerSkins::headsScriptStrings); i++)
			{
				const char* n = Game::SL_ConvertToString(PlayerSkins::headsScriptStrings[i]);
				assert(n);
			}

			for (size_t i = 0; i < ARRAYSIZE(PlayerSkins::bodiesScriptStrings); i++)
			{
				const char* n = Game::SL_ConvertToString(PlayerSkins::bodiesScriptStrings[i]);
				assert(n);
			}
		}

		if (a1 > highest)
		{
			std::ofstream output("test_refcount.txt" , std::ofstream::out | std::ofstream::trunc);

			Logger::Print("========================\n");
			highest = a1;

			for (size_t i = 0; i < a1; i++)
			{
				RefString* ref = reinterpret_cast<RefString*>(&tree->mt_buffer[i * MT_NODE_SIZE]);
				const char* str = ref->str;

				output << std::format("{} => [{}] (refcount: {}  user: {})\n", i, str== nullptr ? "<NULLPTR>" : str, static_cast<int>(ref->___u0.__s0.refCount), static_cast<int>(ref->___u0.__s0.user));
				//Logger::Print("{} => [{}] (refcount: {}  user: {})\n", i, str== nullptr ? "<NULLPTR>" : str, static_cast<int>(ref->___u0.__s0.refCount), static_cast<int>(ref->___u0.__s0.user));
			}
			
			output.flush();
			Logger::Print("========================\n");

		}

		return str;
	}

	void PlayerSkins::RegisterSkins()
	{
		CheckForbiddenHeadBodyCombinations();
		RegisterConstantStrings();
		RefreshPlayerSkinFromDvars();

		initialized = true;

		// Idk what this is, but pass it through
		Utils::Hook::Call<void()>(0x4199D0)();
	}

	PlayerSkins::PlayerSkins()
	{
		//Utils::Hook(0x4C5ECC, SL_ConvertToString, HOOK_CALL).install()->quick();
		//Utils::Hook(0x4C5ED6, SL_ConvertToString, HOOK_CALL).install()->quick();

		Utils::Hook(0x4A76AC, RegisterSkins, HOOK_CALL).install()->quick();

		currentSkin = {};
		currentSkin.enableBody = 1;
		currentSkin.enableHead = 1;

		Utils::Hook(0x41D376, Info_SetValueForKey, HOOK_CALL).install()->quick();

		Utils::Hook(0x4F33D0, GetTrueSkillForGametype, HOOK_JUMP).install()->quick();

		Components::Scheduler::OnGameInitialized([]() {


			}, Components::Scheduler::Pipeline::SERVER
		);

		Components::Events::OnDvarInit([]() {

			localHeadIndexDvar = Dvar::Register(
				"skin_head",
				0,
				0,
				static_cast<int>(ARRAYSIZE(heads) - 1),
				static_cast<uint16_t>(Game::DVAR_SAVED | Game::DVAR_ARCHIVE),
				"custom head index!"
			);

			localBodyIndexDvar = Dvar::Register(
				"skin_body",
				0,
				0,
				static_cast<int>(ARRAYSIZE(bodies) - 1),
				static_cast<uint16_t>(Game::DVAR_SAVED | Game::DVAR_ARCHIVE),
				"custom body index!"
			);

			localEnableHeadDvar = Dvar::Register("skin_enable_head", true, static_cast<uint16_t>(Game::DVAR_SAVED | Game::DVAR_ARCHIVE), "toggle on/off your custom head");
			localEnableBodyDvar = Dvar::Register("skin_enable_body", true, static_cast<uint16_t>(Game::DVAR_SAVED | Game::DVAR_ARCHIVE), "toggle on/off your custom body");

			skinTryOut = Dvar::Register("skin_tryout", false, static_cast<uint16_t>(Game::DVAR_CODINFO), "enable skin tryout mode (refresh skins every second)");

			sv_allowSkins = Dvar::Register("sv_allow_skins", true, static_cast<uint16_t>(Game::DVAR_SAVED | Game::DVAR_ARCHIVE), "allow skins on the server");
			sv_overrideTeamSkins = Dvar::Register("sv_allow_override_team_skins", false, static_cast<uint16_t>(Game::DVAR_SAVED | Game::DVAR_ARCHIVE), "allow body skins in team based gamemodes");

			RefreshPlayerSkinFromDvars();
		});

		Components::GSC::Script::AddMethod("LOUV_GetPlayerSkin", [](const Game::scr_entref_t entref) {
			const auto entity = Game::GetEntity(entref);
			PlayerSkins::GScr_GetPlayerSkin(entity);
		});
	}

	void PlayerSkins::GScr_GetPlayerSkin(Game::gentity_s* entRef)
	{
		if ((*Game::com_sv_running)->current.enabled)
		{
			for (signed int partyIndex = 0; partyIndex < Game::MAX_CLIENTS; partyIndex++)
			{
				if (partyIndex != entRef->client->sess.cs.clientIndex)
				{
					continue;
				}

				const auto client = &Game::svs_clients[partyIndex];
				Skin skin{};

				// global server override
				if (sv_allowSkins.get<bool>() || skinTryOut.get<bool>())
				{
					if (client->header.state == Game::CS_ACTIVE)
					{
						// skin tryout mode - does NOT replicate!!
						if (skinTryOut.get<bool>() && partyIndex == 0)
						{
							RefreshPlayerSkinFromDvars();
							skin = currentSkin;
						}
						else
						{
							const auto strSkinInteger = Game::Info_ValueForKey(client->userinfo, USERINFO_KEY);

							if (std::strlen(strSkinInteger) > 0) {
								skin.intValue = std::stoi(strSkinInteger);
							}

							SanitizeSkin(skin);
						}
					}
					else
					{
						// Still loading or something
					}

					// Returned the array with head first and body last
					Game::Scr_MakeArray();

					std::string errMsg{};
					if (HasAuthorizedBoneCount(skin, errMsg))
					{
						Game::Scr_AddConstString(GetHeadName(skin));
					}
					else
					{
						// Do not put head (too many bones)
						assert(heads[0].empty());
						Game::Scr_AddConstString(headsScriptStrings[0]);

						if (!errMsg.empty())
						{
							Components::Logger::Error(Game::ERR_SCRIPT, errMsg);
						}
					}

					Game::Scr_AddArray();

					Game::Scr_AddConstString(GetBodyName(skin));
					Game::Scr_AddArray();
				}
			}
		}
		else
		{
			const auto err = std::format("Not in a game!!");
			Game::Scr_Error(err.data());
		}
	}
}
