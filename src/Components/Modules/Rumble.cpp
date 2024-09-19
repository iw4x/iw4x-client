#include <STDInclude.hpp>

#include "ConfigStrings.hpp"
#include "Events.hpp"

namespace Components
{
	static Game::RumbleGlobals rumbleGlobArray[Game::MAX_GPAD_COUNT]{}; // We're only gonna use #0 anyway cause only one client

	// Normally these would be defined per-map, but let's just load all of them for good measure
	static const std::string rumbleStrings[] = {
			"riotshield_impact"
			,"damage_heavy"
			,"defaultweapon_fire"
			,"pistol_fire"
			,"defaultweapon_melee"
			,"viewmodel_small"
			,"viewmodel_medium"
			,"viewmodel_large"
			,"silencer_fire"
			,"smg_fire"
			,"assault_fire"
			,"shotgun_fire"
			,"heavygun_fire"
			,"sniper_fire"
			,"artillery_rumble"
			,"grenade_rumble"
			,"ac130_25mm_fire"
			,"ac130_40mm_fire"
			,"ac130_105mm_fire"
			,"minigun_rumble"
	};

	const static Game::cspField_t rumbleFields[4] =
	{
		{"duration", 4, 7},
		{"range", 8, 7},
		{"fadeWithDistance", 0x14, 5},
		{"broadcast", 0x18, 5}
	};

	int GetRumbleInfoIndexFromName(const char* rumbleName)
	{
		for (size_t i = 0; i < Gamepad::RUMBLE_CONFIGSTRINGS_COUNT; i++)
		{
			const std::string& configString = ConfigStrings::CL_GetRumbleConfigString(i);
			if (configString == rumbleName)
			{
				return i;
			}
		}

		return -1;
	}

	Game::ActiveRumble* GetDuplicateRumbleIfExists(Game::cg_s* cgameGlob, Game::ActiveRumble* arArray, Game::RumbleInfo* info, bool loop, Game::RumbleSourceType type, int entityNum, const float* pos)
	{
		assert(cgameGlob);
		assert(arArray);
		assert(type != Game::RUMBLESOURCE_INVALID);

		for (auto i = 0; i < Rumble::MAX_ACTIVE_RUMBLES; i++)
		{
			Game::ActiveRumble*  duplicateRumble = &arArray[i];
			if (duplicateRumble->rumbleInfo != info || duplicateRumble->loop != loop || duplicateRumble->sourceType != type)
				continue;

			bool isSame = false;
			if (type == Game::RUMBLESOURCE_ENTITY)
			{
				isSame = duplicateRumble->source.entityNum == entityNum;
			}
			else
			{
				if (type != Game::RUMBLESOURCE_POS)
					return duplicateRumble;
				if (duplicateRumble->source.pos[0] != *pos || duplicateRumble->source.pos[1] != pos[1])
					continue;
				isSame = duplicateRumble->source.pos[2] == pos[2];
			}

			if (isSame)
				return duplicateRumble;
		};

		return nullptr;
	}

	int FindClosestToDyingActiveRumble(Game::cg_s* cgameGlob, Game::ActiveRumble* activeRumbleArray)
	{
		float oldestRumbleAge = 0.0f;
		int oldestRumbleIndex = 0;
		for (int i = 0; i < Rumble::MAX_ACTIVE_RUMBLES; i++)
		{
			const Game::ActiveRumble* ar = &activeRumbleArray[i];
			assert(ar->rumbleInfo);
			assert(ar->sourceType != Game::RUMBLESOURCE_INVALID);

			if (ar->rumbleInfo && (ar->sourceType != Game::RUMBLESOURCE_ENTITY || ar->source.entityNum != cgameGlob->predictedPlayerState.clientNum))
			{
				auto rumbleInfo = ar->rumbleInfo;
				float timeDiff = static_cast<float>(cgameGlob->time - ar->startTime);
				float timeLived01 = timeDiff / rumbleInfo->duration;
				if (timeLived01 > oldestRumbleAge)
				{
					oldestRumbleIndex = i;
					oldestRumbleAge = timeLived01;
				}
			}
		};

		if (oldestRumbleAge == 0.0f)
		{
			Logger::Warning(Game::CON_CHANNEL_SYSTEM, "FindClosestToDyingActiveRumble(): Couldn't find a suitable rumble to stop, defaulting to index zero.\n");
		}

		return oldestRumbleIndex;
	}

	Game::ActiveRumble* NextAvailableRumble(Game::cg_s* cgameGlob, Game::ActiveRumble* arArray)
	{
		for (auto i = 0; i < Rumble::MAX_ACTIVE_RUMBLES; i++)
		{
			Game::ActiveRumble* candidate = &arArray[i];

			// Extreme guesswork™
			if (candidate->rumbleInfo == nullptr)
			{
				return candidate;
			}

			if (candidate->sourceType == Game::RUMBLESOURCE_INVALID)
			{
				return candidate;
			}

			if (candidate->startTime + candidate->rumbleInfo->duration < cgameGlob->time)
			{
				return candidate;
			}
		};

		auto index = FindClosestToDyingActiveRumble(cgameGlob, arArray);
		assert(index != Rumble::MAX_ACTIVE_RUMBLES);

		return &arArray[index];
	}

	void InvalidateActiveRumble(Game::ActiveRumble* ar)
	{
		ar->sourceType = Game::RUMBLESOURCE_INVALID;
		ar->rumbleInfo = nullptr;
		ar->startTime = -1;
	}

	void CalcActiveRumbles(int localClientNum, Game::ActiveRumble* activeRumbleArray, const float* rumbleReceiverPos)
	{
		auto cg = Game::CL_GetLocalClientGlobals(localClientNum); // CG ?

		float finalRumbleHigh = -1.f;
		float finalRumbleLow = -1.f;
		bool anyRumble = false;

		for (auto i = 0; i < Rumble::MAX_ACTIVE_RUMBLES; i++)
		{
			float scale;

			auto activeRumble = &activeRumbleArray[i];

			if (!activeRumble->rumbleInfo)
			{
				continue;
			}

			assert(activeRumble->sourceType != Game::RUMBLESOURCE_INVALID);


			if (activeRumble->sourceType == Game::RUMBLESOURCE_ENTITY && activeRumble->source.entityNum != cg->predictedPlayerState.clientNum)
			{
				continue;
			}

			{
				float distance = 0.f;

				// Compute rumble distance
				{
					if (activeRumble->sourceType == Game::RUMBLESOURCE_ENTITY)
					{
						auto entity = Game::CG_GetEntity(localClientNum, activeRumble->source.entityNum);
						auto x = (*rumbleReceiverPos - entity->pose.origin[0]);
						auto y = (rumbleReceiverPos[1] - entity->pose.origin[1]);
						auto z = (rumbleReceiverPos[2] - entity->pose.origin[2]);

						distance = std::sqrtf((x * x) + (y * y) + (z * z));

					}
					else
					{
						auto x = (*rumbleReceiverPos - activeRumble->source.pos[0]);
						auto y = (rumbleReceiverPos[1] - activeRumble->source.pos[1]);
						auto z = (rumbleReceiverPos[2] - activeRumble->source.pos[2]);

						distance = std::sqrtf((x * x) + (y * y) + (z * z));
					}
				}

				if (distance <= activeRumble->rumbleInfo->range)
				{
					if (activeRumble->source.entityNum)
					{
						assert(distance > 0.f);

						// Complete guesswork
						scale = 1.f - distance / activeRumble->rumbleInfo->range;
					}
					else
					{
						scale = 1.f;
					}
				}
				else
				{
					continue;
				}
			}

			assert(scale <= 1.f);
			assert(scale >= 0.f);

			// Guesswork
			float duration01 = (cg->time - activeRumble->startTime) / activeRumble->rumbleInfo->duration;
			assert(duration01 >= 0.f);
			assert(duration01 <= 1.f);

			auto highGraph = activeRumble->rumbleInfo->highRumbleGraph;
			auto highValue = Game::GraphGetValueFromFraction(highGraph->knotCount, highGraph->knots, duration01);

			auto lowGraph = activeRumble->rumbleInfo->lowRumbleGraph;
			auto lowValue = Game::GraphGetValueFromFraction(lowGraph->knotCount, lowGraph->knots, duration01);

			finalRumbleHigh = std::max(finalRumbleHigh, highValue * scale);
			finalRumbleLow = std::max(finalRumbleLow, lowValue * scale);

			anyRumble = true;
		}

		if (anyRumble)
		{
			assert(finalRumbleHigh >= 0.F);
			assert(finalRumbleLow >= 0.F);
			Gamepad::GPad_SetHighRumble(localClientNum, finalRumbleHigh);
			Gamepad::GPad_SetLowRumble(localClientNum, finalRumbleLow);
		}
		else
		{
			Gamepad::GPad_SetHighRumble(localClientNum, 0.f);
			Gamepad::GPad_SetLowRumble(localClientNum, 0.f);
		}
	}

	void PlayRumbleInternal(int localClientNum, const char* rumbleName, bool loop, Game::RumbleSourceType type, int entityNum, const float* pos, double scale)
	{
		assert(type != Game::RumbleSourceType::RUMBLESOURCE_INVALID);
		assert(rumbleName);
		assert(*rumbleName);

		int rumbleIndex = GetRumbleInfoIndexFromName(rumbleName);

		if (rumbleIndex < 0)
		{
			Components::Logger::Error(Game::ERR_DROP, "Could not play rumble {} because it was not registered!", rumbleName);
			return;
		}

		auto rumbleInfo = &rumbleGlobArray[localClientNum].infos[rumbleIndex];

		assert(rumbleInfo);

		if (!rumbleInfo->rumbleNameIndex)
		{
			Components::Logger::Error(Game::ERR_DROP, "Could not play rumble {} because it was not registered and loaded. Make sure to precache rumble before playing from script!", rumbleName);
			return;
		}

		auto cg = Game::CL_GetLocalClientGlobals(localClientNum); // should be CG?

		auto activeRumble = GetDuplicateRumbleIfExists(cg, rumbleGlobArray[localClientNum].activeRumbles, rumbleInfo, loop, type, entityNum, pos);

		if (!activeRumble)
		{
			activeRumble = NextAvailableRumble(cg, rumbleGlobArray[localClientNum].activeRumbles);
		}

		assert(activeRumble);

		if (type == Game::RUMBLESOURCE_ENTITY)
		{
			auto entity = Game::CG_GetEntity(localClientNum, entityNum);
			if (!rumbleInfo->broadcast)
			{
				if (!entity->nextValid)
					return;

				if (entity->nextState.eType != 1)
				{
					Logger::Error(
						Game::ERR_FATAL,
						"Non-player entity #{} of type {} at ({}, {}, {}) is trying to play non-broadcasting rumble \"{}\" on themselves.\n",
						entityNum,
						entity->nextState.eType,
						entity->prevState.pos.trBase[0],
						entity->prevState.pos.trBase[1],
						entity->prevState.pos.trBase[2],
						rumbleName);
					return;
				}
			}

			activeRumble->source.entityNum = entityNum;
		}
		else if (type == Game::RUMBLESOURCE_POS)
		{
			std::memcpy(activeRumble->source.pos, pos, ARRAYSIZE(activeRumble->source.pos) * sizeof(float));
		}
		else
		{
			assert(false); // Wrong type
		}

		if (scale < 0.0 || scale > 1.0)
		{
			Logger::Warning(Game::CON_CHANNEL_SYSTEM, "Rumble \"{}\" has invalid scale value of {}.\n", rumbleName, scale);
			scale = 1.0;
		}
		activeRumble->sourceType = type;
		activeRumble->startTime = cg->time;
		activeRumble->rumbleInfo = rumbleInfo;
		activeRumble->loop = loop;
		activeRumble->scale = static_cast<uint8_t>(scale * 255.0);

		if (!cg->nextSnap || cg->predictedPlayerState.clientNum == cg->clientNum && cg->predictedPlayerState.pm_type != 5)
			CalcActiveRumbles(
				localClientNum,
				rumbleGlobArray[localClientNum].activeRumbles,
				rumbleGlobArray[localClientNum].receiverPos);
	}

	void CG_PlayRumbleOnPosition(int localClientNum, const char* rumbleName, const float* pos)
	{
		PlayRumbleInternal(localClientNum, rumbleName, 0, Game::RUMBLESOURCE_POS, 0, pos, 1.0);
	}

	void CG_PlayRumbleOnClient(int localClientNum, const char* rumbleName)
	{
		auto clientGlob = Game::CL_GetLocalClientGlobals(localClientNum);

		assert(clientGlob->nextSnap);

		if (clientGlob->nextSnap)
		{
			PlayRumbleInternal(
				localClientNum,
				rumbleName,
				0,
				Game::RUMBLESOURCE_ENTITY,
				clientGlob->predictedPlayerState.clientNum,
				nullptr,
				1.0);
		}
	}

	void CG_PlayRumbleOnClientSafe(int localClientNum, const char* rumbleName)
	{
		if (GetRumbleInfoIndexFromName(rumbleName) >= 0)
		{
			PlayRumbleInternal(localClientNum, rumbleName, 0, Game::RUMBLESOURCE_ENTITY, Game::CL_GetLocalClientGlobals(localClientNum)->predictedPlayerState.clientNum, 0, 1.0);
		}
		else
		{
			Game::Com_PrintWarning(14, "Can't play rumble asset '%s' because it is not registered.", rumbleName);
		}
	}

	void Rumble_Strcpy(char* member, char* keyValue)
	{
		strcpy(member, keyValue);
	}

	bool ParseRumbleGraph(Game::RumbleGraph* graph, const char* buffer, const char* fileName)
	{
#define MAX_RUMBLE_GRAPH_KNOTS 16

		auto buffer_ = buffer;
		assert(graph);
		Game::Com_BeginParseSession(fileName);
		auto knotCountStr = Game::Com_Parse(&buffer_);
		auto parsedKnotCount = atoi(knotCountStr);
		if (parsedKnotCount <= MAX_RUMBLE_GRAPH_KNOTS)
		{
			if (parsedKnotCount >= 0)
			{
				graph->knotCount = static_cast<unsigned short>(parsedKnotCount);

				if (graph->knotCount)
				{
					for (auto i = 0; i < graph->knotCount; i++)
					{
						auto knot = &graph->knots[i];

						const char* parsedCharacterA = Game::Com_Parse(&buffer_);
						if (!*parsedCharacterA)
							break;
						if (*parsedCharacterA == '}')
							break;

						float floatA = static_cast<float>(atof(parsedCharacterA));

						const char* parsedCharacterB = Game::Com_Parse(&buffer_);
						if (!*parsedCharacterB || *parsedCharacterB == '}')
							break;
						float floatB = static_cast<float>(atof(parsedCharacterB));

						if (i >= MAX_RUMBLE_GRAPH_KNOTS)
						{
							Logger::Error(Game::ERR_DROP, "knotCountIndex doesn't index MAX_RUMBLE_GRAPH_KNOTS: {} not in [0, {}])", i, MAX_RUMBLE_GRAPH_KNOTS);
						}

						(*knot)[0] = floatA;
						(*knot)[1] = floatB;
					};
				}

				Game::Com_EndParseSession();

				return true;
			}
			else
			{
				Game::Com_EndParseSession();
				Logger::Error(Game::ERR_DROP, "Negative graph nots on {}", fileName);
				return false;
			}
		}
		else
		{
			Game::Com_EndParseSession();
			Logger::Error(Game::ERR_DROP, "Too many graph nots on {}", fileName);
			return false;
		}
	}

	void ReadRumbleGraph(Game::RumbleGraph* graph, const char* rumbleFileName)
	{
		assert(graph);
		assert(rumbleFileName);

		char buff[256]{};
		std::string path = std::format("rumble/{}", rumbleFileName);

		[[maybe_unused]] auto graphBefore = graph;

		strncpy(graph->graphName, rumbleFileName, 64);
		auto data = Game::Com_LoadInfoString(path.data(), "rumble graph file", "RUMBLEGRAPHFILE", buff);
		//auto data = Utils::Hook::Call<char*(const char*, const char*, const char*, char*)>(0x463500)(path.data(), "rumble graph file", "RUMBLEGRAPHFILE", buff);

		assert(graph == graphBefore);

		graph->knotCount = 0;
		if (!ParseRumbleGraph(graph, data, rumbleFileName))
		{
			Logger::Error(Game::ERR_DROP, "Error in parsing rumble file {}", rumbleFileName);
		}

	}

	int LoadRumbleGraph(Game::RumbleGraph* rumbleGraphArray, Game::RumbleInfo* info, const char* highRumbleFileName, const char* lowRumbleFileName)
	{
		info->highRumbleGraph = 0;
		info->lowRumbleGraph = 0;

		auto i = 0;

		for (i = 0; i < 64; ++i)
		{
			auto rumbleGraph = &rumbleGraphArray[i];
			if (!rumbleGraph->knotCount)
				break;
			if (!_strnicmp(rumbleGraph->graphName, highRumbleFileName, 0x7FFFFFFF)) // TODO change that
				info->highRumbleGraph = rumbleGraph;
			if (!_strnicmp(rumbleGraph->graphName, lowRumbleFileName, 0x7FFFFFFF))
				info->lowRumbleGraph = rumbleGraph;
		}
		if (!info->highRumbleGraph || !info->lowRumbleGraph)
		{
			if (i == 64)
				Components::Logger::Error(Game::ERR_DROP, "No more room to allocate rumble graph");

			auto rumbleGraph = &rumbleGraphArray[i];

			while (i < 64)
			{
				if (i == 64)
				{
					Components::Logger::Error(Game::ERR_DROP, "No more room to allocate rumble graph");
				}
				else if (!info->highRumbleGraph)
				{
					ReadRumbleGraph(rumbleGraph, highRumbleFileName);
					info->highRumbleGraph = rumbleGraph;
					i++;
				}
				else if (!info->lowRumbleGraph)
				{
					ReadRumbleGraph(rumbleGraph, lowRumbleFileName);
					info->lowRumbleGraph = rumbleGraph;
					i++;
				}
				else
				{
					break;
				}
			}

			// There's more stuff that should be happening here
		}

		return 1;
	}

	int CG_LoadRumble(Game::RumbleGraph* rumbleGraphArray, Game::RumbleInfo* info, const char* rumbleName, int rumbleNameIndex)
	{
		assert(info);
		assert(rumbleName);

		std::string path = std::format("rumble/{}", rumbleName);
		char buff[256]{}; // should be 64 but it ALWAYS goes overboard!

		[[maybe_unused]] auto infoPtr = info;
		const char* str = Game::Com_LoadInfoString(path.data(), "rumble info file", "RUMBLE", buff); 
		assert(infoPtr == info);

		const std::string highRumbleFile = Game::Info_ValueForKey(str, "highRumbleFile");
		const std::string lowRumbleFile = Game::Info_ValueForKey(str, "lowRumbleFile");

		if (!Game::ParseConfigStringToStruct(info, rumbleFields, 4, str, 0, 0, Rumble_Strcpy))
		{
			return 0;
		}

		if (info->broadcast)
		{
			if (info->range == 0.0)
			{
				Components::Logger::Error(Game::ERR_DROP, "Rumble file {} cannot have broadcast because its range is zero\n", rumbleName);
			}
		}

		if (!LoadRumbleGraph(rumbleGraphArray, info, highRumbleFile.data(), lowRumbleFile.data()))
			return 0;

		info->rumbleNameIndex = rumbleNameIndex;
		info->duration = info->duration * 1000.f;

		return 1;
	}

	void CG_RegisterRumbles(int localClientNum)
	{
		const auto myRumbleGlobal = &rumbleGlobArray[localClientNum];
		const auto maxRumbleGraphIndex = Gamepad::RUMBLE_CONFIGSTRINGS_COUNT;

		for (int i = 1; i < maxRumbleGraphIndex; i++)
		{
			auto rumbleConf = ConfigStrings::CL_GetRumbleConfigString(i);
			if (*rumbleConf)
			{
				CG_LoadRumble(myRumbleGlobal->graphs, &rumbleGlobArray[localClientNum].infos[i-1], rumbleConf, i);
			}
		}
	}

	void CG_RegisterGraphics_Hk(int localClientNum, int b)
	{
		// Call original function
		Utils::Hook::Call<void(int, int)>(0x5895D0)(localClientNum, b);

		CG_RegisterRumbles(localClientNum);

		printf("");
	}

	void G_RumbleIndex(const char* name)
	{
		assert(name);

		if (*name)
		{
			auto v2 = Game::SL_FindLowercaseString(name);
			int i;

			for (i = 1; i <= Gamepad::RUMBLE_CONFIGSTRINGS_COUNT; ++i)
			{
				auto v7 = ConfigStrings::SV_GetRumbleConfigStringConst(i);
				if (v7 == Game::scr_const->_)
					break;
				if (v7 == v2)
					return;
			}

			if (i <= Gamepad::RUMBLE_CONFIGSTRINGS_COUNT)
			{
				Logger::Print("WARNING: Rumble not registered, {}\n", name);
			}
			else
			{
				ConfigStrings::SV_SetRumbleConfigString(i, name);
			}
		}
	}

	void RegisterWeaponRumbles(Game::WeaponDef* weapDef)
	{
		assert(weapDef);

		auto fireRumble = weapDef->fireRumble;
		if (fireRumble && *fireRumble)
			G_RumbleIndex(fireRumble);

		auto meleeImpactRumble = weapDef->meleeImpactRumble;
		if (meleeImpactRumble && *meleeImpactRumble)
			G_RumbleIndex(meleeImpactRumble);

		auto turretBarrelSpinRumble = weapDef->turretBarrelSpinRumble;
		if (turretBarrelSpinRumble && *turretBarrelSpinRumble)
			G_RumbleIndex(turretBarrelSpinRumble);


		//Disabled for now cause it's weird
		for (auto i = 0; i < 16; ++i)
		{
			if (!weapDef->notetrackRumbleMapKeys[i])
				break;
			auto noteTrackRumbleMap = weapDef->notetrackRumbleMapValues;
			if (noteTrackRumbleMap[i])
			{
				auto str = Game::SL_ConvertToString(noteTrackRumbleMap[i]);
				G_RumbleIndex(str);
			}
		}
	}

	void CG_FireWeapon_Rumble(int localClientNum, Game::entityState_s* ent, Game::WeaponDef* weaponDef, bool isPlayerView)
	{
		assert(ent);
		assert(weaponDef);

		int v10;

		if (weaponDef)
		{
			auto rumbleName = weaponDef->fireRumble;
			if (rumbleName && *rumbleName)
			{
				auto cg = Game::CL_GetLocalClientGlobals(localClientNum); // should be CG instead

				if (ent->eType != 12
					|| (cg->predictedPlayerState.eFlags & Game::EF_VEHICLE_ACTIVE) == 0
					|| (v10 = 1, cg->predictedPlayerState.viewlocked_entNum != ent->number))
				{
					v10 = 0;
				}
				if (isPlayerView || v10)
					CG_PlayRumbleOnClient(localClientNum, weaponDef->fireRumble);
			}
		}
	}

	void __declspec(naked) CG_FireWeapon_FireSoundHk()
	{
		__asm
		{
			pushad;

				push bx
				push[esp + 0x20 + 0x28 + 0x2] // weapon
				push esi // cent
				push ebp

				call CG_FireWeapon_Rumble

				add esp, 0x4 * 3 + 0x2

			popad;

				// OG code
				sub esp, 0x10;
				push ebp;
				mov ebp, [esp + 0x24];

			// Return
			push 0x59D7D8;
			retn;
		}
	}

	Game::WeaponDef* BG_GetWeaponDef_RegisterRumble_Hk(unsigned int weapIndex)
	{
		auto weapDef = Game::BG_GetWeaponDef(weapIndex);

		RegisterWeaponRumbles(weapDef);

		return weapDef;
	}

	void SCR_UpdateRumble()
	{
		auto connectionState = Game::CL_GetLocalClientConnectionState(0);

		int controllerIndex = Game::CL_ControllerIndexFromClientNum(0);
		if (connectionState != 9 || (*Game::cl_paused)->current.enabled)
			controllerIndex = -1;
		else
			Gamepad::GPad_UpdateRumble(controllerIndex);

		for (auto i = 0; i < Game::MAX_GPAD_COUNT; ++i)
		{
			if (i != controllerIndex)
				Gamepad::GPad_StopRumbles(i);
		}
	}

	void RemoveInactiveRumbles(int localClientNum, Game::ActiveRumble* activeRumbleArray)
	{
		auto cg = Game::CL_GetLocalClientGlobals(localClientNum);

		for (int i = 0; i < Rumble::MAX_ACTIVE_RUMBLES; i++)
		{
			auto ar = &activeRumbleArray[i];

			if (!ar->rumbleInfo)
			{
				continue;
			}

			assert(ar->sourceType != Game::RUMBLESOURCE_INVALID);

			// This is not what the game does but... it sounds logical
			if (ar->rumbleInfo->duration < cg->time - ar->startTime)
			{
				InvalidateActiveRumble(ar);
				continue;
			}

			if (ar->sourceType == Game::RUMBLESOURCE_ENTITY && ar->source.pos)
			{
				auto entity = Game::CG_GetEntity(localClientNum, ar->source.entityNum);
				auto snap = &cg->predictedPlayerState;
				auto eFlags =
					ar->source.entityNum == snap->clientNum ?
					snap->eFlags :
					entity->nextState.lerp.eFlags;


				// EF_LOOP_RUMBLE Seems to never be set
				// I don't know where to look for it
				// So we need to comment it out in the meantime otherwise no rumble ever plays
				if (!entity->nextValid/* || (eFlags & Game::EF_LOOP_RUMBLE) == 0*/)
				{
					ar->sourceType = Game::RUMBLESOURCE_INVALID;
					ar->rumbleInfo = nullptr;
					ar->startTime = -1;
					continue;
				}
			}
		}
	}

	void CG_UpdateRumble(int localClientNum)
	{
		auto cg = Game::CL_GetLocalClientGlobals(localClientNum);
		if (cg->nextSnap && (cg->predictedPlayerState.clientNum != cg->clientNum || cg->predictedPlayerState.pm_type == 5))
		{
			for (int i = 0; i < Rumble::MAX_ACTIVE_RUMBLES; i++)
			{
				auto ar = &rumbleGlobArray[localClientNum].activeRumbles[i];

				if (ar->startTime < 0)
				{
					break;
				}

				InvalidateActiveRumble(ar);
			}

			Gamepad::GPad_SetLowRumble(localClientNum, 0.0);
			Gamepad::GPad_SetHighRumble(localClientNum, 0.0);
		}
		else
		{
			RemoveInactiveRumbles(localClientNum, rumbleGlobArray[localClientNum].activeRumbles);
			CalcActiveRumbles(localClientNum, rumbleGlobArray[localClientNum].activeRumbles, rumbleGlobArray[localClientNum].receiverPos);
		}
	}

	void CG_SetRumbleReceiver()
	{
		constexpr int localClientIndex = 0; // :(

		rumbleGlobArray[localClientIndex].receiverEntNum = Game::CL_GetLocalClientGlobals(localClientIndex)->predictedPlayerState.clientNum;

		std::memcpy(
			rumbleGlobArray[localClientIndex].receiverPos,
			//Game::CL_GetLocalClientGlobals(localClientIndex)->refdef.view.org,
			Game::CG_GetEntity(localClientIndex, rumbleGlobArray[localClientIndex].receiverEntNum)->pose.origin,
			sizeof(float) * ARRAYSIZE(rumbleGlobArray[localClientIndex].receiverPos
			)
		);

		// R_EndDobjScene
		Utils::Hook::Call<void()>(0x50BB30)();
	}

	void CG_UpdateEntInfo_Hk()
	{
		Utils::Hook::Call<void()>(0X5994B0)(); // Call original
		CG_UpdateRumble(0); // Local client has to be zero i guess :<
	}

	void DebugRumbles()
	{
		Game::Font_s* font = Game::R_RegisterFont("fonts/smallFont", 0);
		auto height = Game::R_TextHeight(font);
		auto scale = 0.55f;

		auto activeRumbles = rumbleGlobArray[0].activeRumbles;

		for (std::size_t i = 0; i < Rumble::MAX_ACTIVE_RUMBLES; ++i)
		{
			float color[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
			std::stringstream str;
			str << std::format("{} => ", i);

			if (activeRumbles[i].rumbleInfo == nullptr)
			{
				str << "INACTIVE";
			}
			else
			{
				auto activeRumble = &activeRumbles[i];
				auto cg = Game::CL_GetLocalClientGlobals(0); // CG ?
				float duration01 = (cg->time - activeRumble->startTime) / activeRumble->rumbleInfo->duration;

				auto highGraph = activeRumble->rumbleInfo->highRumbleGraph;
				auto highValue = Game::GraphGetValueFromFraction(highGraph->knotCount, highGraph->knots, duration01);

				auto lowGraph = activeRumble->rumbleInfo->lowRumbleGraph;
				auto lowValue = Game::GraphGetValueFromFraction(lowGraph->knotCount, lowGraph->knots, duration01);

				str << std::format("HIGH: {} / LOW: {} (Time left: {:.0f}%)", highValue * scale, lowValue * scale, duration01 * 100);

				color[0] = 0.f;
				color[2] = 1.f;
			}

			Game::R_AddCmdDrawText(str.str().data(), std::numeric_limits<int>::max(), font, 15.0f, (height * scale + 1) * (i + 1) + 4.0f, scale, scale, 0.0f, color, Game::ITEM_TEXTSTYLE_NORMAL);
		}
	}
	

	void LoadConstantRumbleConfigStrings()
	{
		static_assert(ARRAYSIZE(rumbleStrings) < Gamepad::RUMBLE_CONFIGSTRINGS_COUNT);

		for (size_t i = 0; i < ARRAYSIZE(rumbleStrings); i++)
		{
			// this registers the config string as constant
			ConfigStrings::SV_SetRumbleConfigString(i+1, rumbleStrings[i].data());
		}
	}

	int CCS_GetChecksum_Hk()
	{
		LoadConstantRumbleConfigStrings();
		return Utils::Hook::Call<int()>(0x4A0060)();
	}

	void SV_InitGameProgs_Hk(int arg)
	{
		LoadConstantRumbleConfigStrings();

		return Utils::Hook::Call<void(int)>(0x445940)(arg);
	}

	void CG_GetImpactEffectForWeapon_Hk(int localClientNum, const int sourceEntityNum, const int weaponIndex, const int surfType, const int impactFlags, Game::FxEffectDef** outFx, Game::snd_alias_list_t** outSnd)
	{
		CG_PlayRumbleOnClient(localClientNum, "riotshield_impact");
		Utils::Hook::Call<void(int, int, int, int, int, Game::FxEffectDef**, Game::snd_alias_list_t**)>(0x4E43E0)(localClientNum, sourceEntityNum, weaponIndex, surfType, impactFlags, outFx, outSnd);
	}

	void CG_ExplosiveImpactOnShieldEvent(int localClientNum)
	{
		CG_PlayRumbleOnClient(localClientNum, "riotshield_impact");
		Utils::Hook::Call<void(int)>(0x4FBCB0)(localClientNum);
	}

	void CG_ExplosiveSplashOnShieldEvent(int localClientNum, int weaponIndex)
	{
		CG_PlayRumbleOnClient(localClientNum, "riotshield_impact");
		Utils::Hook::Call<void(int, int)>(0x4F2EA0)(localClientNum, weaponIndex);
	}

	void PlayNoteMappedRumbleAliases(int localClientNum, const char* noteName, Game::WeaponDef* weapDef)
	{
		if (*weapDef->notetrackRumbleMapKeys)
		{
			const auto stringID = Game::SL_FindLowercaseString(noteName);
			if (stringID)
			{
				for (auto i = 0; i < 16; ++i)
				{
					if (!weapDef->notetrackRumbleMapKeys[i])
						break;

					const auto values = weapDef->notetrackRumbleMapValues;
					if (values[i] && weapDef->notetrackRumbleMapKeys[i] == stringID)
					{
						const auto rumbleName = Game::SL_ConvertToString(values[i]);
						if (rumbleName)
						{
							CG_PlayRumbleOnClientSafe(localClientNum, rumbleName);
						}
					}
				}
			}
		}
	}

	void __declspec(naked) PlayNoteMappedSoundAliases_Stub()
	{
		__asm
		{
			pushad
				push edi // WeapDeff
				push ecx // NoteName
				push edx // LocalClientNum

				call PlayNoteMappedRumbleAliases

				pop edx
				pop ecx
				pop edi
			popad

			// original code
			mov eax, [edi + 0x18]
			cmp word ptr[eax], 0

			// Go back
			push 0x59C447
			retn
		}
	}

	Rumble::Rumble()
	{
		if (ZoneBuilder::IsEnabled())
			return;

		// CG_setRumbleReceiver
		Utils::Hook(0x486DEC, CG_SetRumbleReceiver, HOOK_CALL).install()->quick();

		// Client & server
		Utils::Hook(0x5AC2F3, CCS_GetChecksum_Hk, HOOK_CALL).install()->quick();
		Utils::Hook(0x4A75CC, SV_InitGameProgs_Hk, HOOK_CALL).install()->quick();

		// Rumble action
		Utils::Hook(0x59D7D0, CG_FireWeapon_FireSoundHk, HOOK_JUMP).install()->quick();

		// CG_BulletHitClientShield
		Utils::Hook(0x42A611, CG_GetImpactEffectForWeapon_Hk, HOOK_CALL).install()->quick();

		//  CG_ExplosiveImpactOnShield
		Utils::Hook(0x4DE156, CG_ExplosiveImpactOnShieldEvent, HOOK_CALL).install()->quick();

		// CG_ExplosiveSplashOnShieldEvent
		Utils::Hook(0x4DE17D, CG_ExplosiveSplashOnShieldEvent, HOOK_CALL).install()->quick();

		// PlayNoteMappedRumbleAliases
		Utils::Hook(0x59C440, PlayNoteMappedSoundAliases_Stub, HOOK_JUMP).install()->quick();

		// TODO: CG_PlayRumbleOnEntity
		// TODO: CG_PlayRumbleOnPosition
		// TODO: CG_PlayRumbleLoopOnEntity
		// TODO: CG_PlayRumbleLoopOnPosition
		// TODO: CG_PlayRumbleOnClientScaledWithUpdate
		// TODO: CG_PlayLoopRumbleOnClient
		// TODO: CG_StopRumble
		// TODO: CG_StopAllRumbles

		// TODO: ScrCmd_PlayRumbleOnEntity_Internal
		// TODO: ScrCmd_PlayRumbleOnEntity
		// TODO: ScrCmd_PlayRumbleLoopOnEntity

		// TODO: G_InitDefaultViewmodelRumbles
		// TODO: CG_Turret_UpdateBarrelSpinRumble


		// Frame rumble update
		Utils::Hook(0x47E035, SCR_UpdateRumble, HOOK_CALL).install()->quick();
		Utils::Hook(0x486BB6, CG_UpdateEntInfo_Hk, HOOK_CALL).install()->quick();


		// rumble loading
		Utils::Hook(0x43E1F8, BG_GetWeaponDef_RegisterRumble_Hk, HOOK_CALL).install()->quick();
		Utils::Hook(0x4E37D3, CG_RegisterGraphics_Hk, HOOK_CALL).install()->quick();

		// CG_PlayRumble_f
		Command::Add("playrumble", [](const Command::Params* params) {
			if (Game::CL_GetLocalClientGlobals(0)->nextSnap)
			{
				if (params->size() == 2)
				{
					auto rumbleName = params->get(1);
					CG_PlayRumbleOnClient(0, rumbleName);
				}
				else
				{
					Game::Com_Printf(0, "USAGE: playrumble <rumblename>\n");
				}
			}
			});

		// Debug
		Scheduler::Loop([]() {
			DebugRumbles();
			}, Scheduler::Pipeline::RENDERER);

	}

}
