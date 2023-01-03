#include <STDInclude.hpp>


namespace Components
{


	static Game::RumbleGlobals rumbleGlobArray[Game::MAX_GPAD_COUNT]{}; // We're only gonna use #0 anyway cause only one client
	static constexpr unsigned int MAX_ACTIVE_RUMBLES = 32;

	int GetRumbleInfoIndexFromName(const char* rumbleName)
	{

		int i = 1;
		int v7;
		while (1)
		{
			const char* configString = Game::CL_GetConfigString(i + 1024);
			auto rumbleName_ = rumbleName;

			do
			{
				char currentCStringChar = *configString;
				bool hasReachedEnd = currentCStringChar == 0;
				v7 = currentCStringChar - *rumbleName_;
				if (hasReachedEnd)
					break;
				++configString;
				++rumbleName_;
			} while (!v7);

			if (!v7)
				break;
			if (++i >= MAX_ACTIVE_RUMBLES)
				return -1;
		}
		return i;
	}

	Game::ActiveRumble* GetDuplicateRumbleIfExists(Game::cg_s* cgameGlob, Game::ActiveRumble* arArray, Game::RumbleInfo* info, bool loop, Game::RumbleSourceType type, int entityNum, const float* pos)
	{
		Game::ActiveRumble* result; // r3

		assert(cgameGlob);
		assert(arArray);
		assert(type != Game::RUMBLESOURCE_INVALID);

		for (auto i = 0; i < MAX_ACTIVE_RUMBLES; i++)
		{
			result = &arArray[i];
			if (result->rumbleInfo != info || result->loop != loop || result->sourceType != type)
				continue;

			bool isSame = false;
			if (type == Game::RUMBLESOURCE_ENTITY)
			{
				isSame = result->source.entityNum == entityNum;
			}
			else
			{
				if (type != Game::RUMBLESOURCE_POS)
					return result;
				if (result->source.pos[0] != *pos || result->source.pos[1] != pos[1])
					continue;
				isSame = result->source.pos[2] == pos[2];
			}

			if (isSame)
				return result;
		};

		return 0;
	}

	int FindClosestToDyingActiveRumble(Game::cg_s* cgameGlob, Game::ActiveRumble* activeRumbleArray)
	{
		__int64 v2; // r7
		double v3; // fp31
		int v5; // r23
		int v6; // r28
		Game::RumbleSourceType* v7; // r31
		int v8; // r9

		v3 = 0.0;
		v5 = 32;
		v6 = 0;
		for (int i = 0; i < MAX_ACTIVE_RUMBLES; i++)
		{
			Game::ActiveRumble* ar = &activeRumbleArray[i];
			assert(ar->rumbleInfo);
			assert(ar->sourceType != Game::RUMBLESOURCE_INVALID);

			if (ar->rumbleInfo && (ar->sourceType != Game::RUMBLESOURCE_ENTITY || ar->source.entityNum != cgameGlob->predictedPlayerState.clientNum))
			{
				auto rumbleInfo = ar->rumbleInfo;
				double timeDiff = cgameGlob->time - ar->startTime;
				if ((timeDiff / rumbleInfo->duration) > v3)
				{
					v5 = v6;
					v3 = (timeDiff / rumbleInfo->duration);
				}
			}
			++v6;
		};

		if (v5 != 32)
			return v5;

		Logger::Warning(Game::CON_CHANNEL_SYSTEM, "FindClosestToDyingActiveRumble(): Couldn't find a suitable rumble to stop, defaulting to index zero.\n");
		return 0;
	}

	Game::ActiveRumble* NextAvailableRumble(Game::cg_s* cgameGlob, Game::ActiveRumble* arArray)
	{
		int v4; // r9
		int* v5; // r11
		int v7; // r31

		for (auto i = 0; i < MAX_ACTIVE_RUMBLES; i++)
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
		assert(index != MAX_ACTIVE_RUMBLES);

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

		for (auto i = 0; i < MAX_ACTIVE_RUMBLES; i++)
		{
			float scale;

			auto activeRumble = &activeRumbleArray[i];

			if (!activeRumble->rumbleInfo)
			{
				continue;
			}

			assert(activeRumble->sourceType != Game::RUMBLESOURCE_INVALID);

			if (activeRumble->rumbleInfo->fadeWithDistance) { // I think ?

				if (activeRumble->sourceType == Game::RUMBLESOURCE_ENTITY && activeRumble->source.entityNum != cg->predictedPlayerState.clientNum)
				{
					continue;
				}

				scale = 1.f; // ???
			}
			else
			{
				float distance = 0.f;
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
					/*	if (v16 != RUMBLESOURCE_POS)
							MyAssertHandler(
								"c:\\trees\\iw4build1-iw\\iw4\\code_source\\src\\cgame\\cg_rumble.cpp",
								591,
								0,
								"%s",
								"ar->sourceType == RUMBLESOURCE_POS");
						v26 = (*rumbleReceiverPos - *(v12 + 2));
						v27 = (rumbleReceiverPos[2] - *(v12 + 4));
						v24 = (rumbleReceiverPos[1] - *(v12 + 3));
						v25 = ((v27 * v27) + (v26 * v26));*/
				}

				if (distance <= activeRumble->rumbleInfo->range)
				{
					if (activeRumble->source.entityNum)
					{
						assert(distance > 0.f);

						// Complete guesswork
						scale = 1.f - distance / activeRumble->rumbleInfo->range;
					}
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
			Components::Logger::Error(Game::ERR_DROP, "Could not play rumble {} because it was not registered and loaded. Make surez to precache rumble before playing from script!", rumbleName);
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
			//if (!rumbleInfo->broadcast)
			//{
			//	if ((entity->flags & 1) == 0)
			//		return;
			//	if (entity->nextState.eType != 1)
			//	{
			//		Logger::Error(
			//			Game::ERR_FATAL,
			//			"Non-player entity #{} of type {} at ({}, {}, {}) is trying to play non-broadcasting rumble \"{}\" on themselves.\n",
			//			entityNum,
			//			entity->nextState.eType,
			//			entity->prevState.pos.trBase[0],
			//			entity->prevState.pos.trBase[1],
			//			entity->prevState.pos.trBase[2],
			//			rumbleName);
			//		return;
			//	}
			//}

			activeRumble->source.entityNum = entityNum;
		}
		else if (type == Game::RUMBLESOURCE_POS)
		{
			// Not implemented for now
			printf("");
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
		activeRumble->scale = (scale * 255.0);

		if (!cg->nextSnap || cg->predictedPlayerState.clientNum == cg->clientNum && cg->predictedPlayerState.pm_type != 5)
			CalcActiveRumbles(
				localClientNum,
				rumbleGlobArray[localClientNum].activeRumbles,
				rumbleGlobArray[localClientNum].receiverPos);
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



	static Game::cspField_t rumbleFields[4] =
	{
		{"duration", 4, 7},
		{"range", 8, 7},
		{"fadeWithDistance", 0x14, 5},
		{"broadcast", 0x18, 5}
	};

	void Rumble_Strcpy(char* member, char* keyValue)
	{
		int v2; // r11
		char v3; // r10

		v2 = member - keyValue;
		do
		{
			v3 = *keyValue;
			(keyValue++)[v2] = v3;
		} while (v3);
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

						float floatA = atof(parsedCharacterA);

						const char* parsedCharacterB = Game::Com_Parse(&buffer_);
						if (!*parsedCharacterB || *parsedCharacterB == '}')
							break;
						float floatB = atof(parsedCharacterB);

						if (i >= MAX_RUMBLE_GRAPH_KNOTS)
						{
							Logger::Error(Game::ERR_DROP, "knotCountIndex doesn't index MAX_RUMBLE_GRAPH_KNOTS: {} not in [0, {}])", i, MAX_RUMBLE_GRAPH_KNOTS);
						}

						(*knot)[0] = floatA;
						(*knot)[1] = floatB;
					};
				}
				Game::Com_EndParseSession();

				///
				if (graph->graphName == "smg_fire_l.rmb"s)
				{
					printf("");
				}
				///

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
		auto highRumbleFileName_ = highRumbleFileName;
		info->lowRumbleGraph = 0;
		auto lowRumbleFileName_ = lowRumbleFileName;

		auto i = 0;

		for (i = 0; i < 64; ++i)
		{
			auto rumbleGraph = &rumbleGraphArray[i];
			if (!rumbleGraph->knotCount)
				break;
			if (!strnicmp(rumbleGraph->graphName, highRumbleFileName, 0x7FFFFFFF)) // TODO change that
				info->highRumbleGraph = rumbleGraph;
			if (!strnicmp(rumbleGraph->graphName, lowRumbleFileName_, 0x7FFFFFFF))
				info->lowRumbleGraph = rumbleGraph;
		}
		if (!info->highRumbleGraph || !info->lowRumbleGraph)
		{
			if (i == 64)
				Components::Logger::Error(Game::ERR_DROP, "No moore room to allocate rumble graph");

			auto rumbleGraph = &rumbleGraphArray[i];

			while (i < 64)
			{
				if (i == 64)
				{
					Components::Logger::Error(Game::ERR_DROP, "No moore room to allocate rumble graph");
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

			printf("");
			//int v12;
			//int v15;
			//unsigned short* v16 = nullptr;
			//if (!info->highRumbleGraph || !info->lowRumbleGraph)
			//{
			//	v15 = 0;
			//	v16 = &rumbleGraphArray[1].knotCount;
			//	do
			//	{
			//		*(&v12 + 1) = *(v16 - 118);
			//		if (!*(v16 - 118))
			//			break;
			//		*(&v12 + 1) = *v16;
			//		if (!*v16)
			//		{
			//			++v15;
			//			break;
			//		}
			//		*(&v12 + 1) = v16[118];
			//		if (!v16[118])
			//		{
			//			v15 += 2;
			//			break;
			//		}
			//		*(&v12 + 1) = v16[236];
			//		if (!v16[236])
			//		{
			//			v15 += 3;
			//			break;
			//		}
			//		v15 += 4;
			//		v16 += 472;
			//	} while (v15 < 64);
			//	if (v15 == 64)
			//		Components::Logger::Error(Game::ERR_DROP, "No moore room to allocate rumble graph");
			////	
			//	auto v17 = &rumbleGraphArray[v15];
			//	ReadRumbleGraph(v17, lowRumbleFileName);
			//	info->lowRumbleGraph = v17;
			//}
		}

		return 1;
	}

	int CG_LoadRumble(Game::RumbleGraph* rumbleGraphArray, Game::RumbleInfo* info, const char* rumbleName, int rumbleNameIndex)
	{
		assert(info);
		assert(rumbleName);

		///
		if (rumbleName == "weap_m9_clipout_plr"s)
		{
			printf("");
		}
		///

		std::string path = std::format("rumble/{}", rumbleName);
		char buff[256]{}; //64

		[[maybe_unused]] auto infoPtr = info;
		const char* str = Game::Com_LoadInfoString(path.data(), "rumble info file", "RUMBLE", buff); // Idk why but this destroys the stack
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
		info->duration = info->duration * 1000.0;

		return 1;
	}

	void CG_RegisterRumbles(int localClientNum)
	{
		auto configStringBasePos = 1025;
		auto myRumbleGlobal = &rumbleGlobArray[localClientNum];
		auto maxRumbleGraphIndex = 31;
		auto myRumbleInfo = &rumbleGlobArray[localClientNum].infos[1];
		do
		{
			auto rumbleConf = Game::CL_GetConfigString(configStringBasePos);
			if (*rumbleConf)
			{
				CG_LoadRumble(myRumbleGlobal->graphs, myRumbleInfo, rumbleConf, configStringBasePos);
			}

			--maxRumbleGraphIndex;
			++configStringBasePos;
			++myRumbleInfo;
		} while (maxRumbleGraphIndex);
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
			///
			if (name == "weap_m9_clipout_plr"s)
			{
				printf("");
			}
			///
			auto v2 = Game::SL_FindLowercaseString(name);
			int i;

			for (i = 1; i < MAX_ACTIVE_RUMBLES; ++i)
			{
				auto v7 = Game::SV_GetConfigstringConst(i + 1024);
				if (v7 == Game::scr_const->_)
					break;
				if (v7 == v2)
					return;
			}

			Game::SV_SetConfigstring(i + 1024, name);
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
			//
			weapDef->notetrackRumbleMapKeys[i] = 0;
			//

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
					|| (cg->predictedPlayerState.eFlags & 0x100000) == 0
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
		static auto CG_FireWeapon_FireSound_t = 0x59D7D0u;

		__asm
		{
			call CG_FireWeapon_FireSound_t;

			pushad;
			//CG_FireWeapon_Rumble(localClientNum, cent, weapon, isPlayerView);

			push bl
			push[esp + 0x20 + 0x58] // weapon
			push[esp + 0x20 + 0x4] // cent
			push ebp

			call CG_FireWeapon_Rumble

			add esp, 0x4 * 4

			popad;

			push 0x4FB362;
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

		for (int i = 0; i < MAX_ACTIVE_RUMBLES; i++)
		{
			auto ar = &rumbleGlobArray[localClientNum].activeRumbles[i];

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
				auto snap = cg->nextSnap;
				auto eFlags = ar->source.entityNum == snap->ps.clientNum ? snap->ps.eFlags : entity->nextState.lerp.eFlags;
				//if (!entity->nextValid || (eFlags & 0x1000) == 0)
				//{
				//	ar->sourceType = Game::RUMBLESOURCE_INVALID;
				//	ar->rumbleInfo = nullptr;
				//	ar->startTime = -1;
				//	continue;
				//}
			}
		}
	}

	void CG_UpdateRumble(int localClientNum)
	{
		auto cg = Game::CL_GetLocalClientGlobals(localClientNum);
		if (cg->nextSnap && (cg->predictedPlayerState.clientNum != cg->clientNum || cg->predictedPlayerState.pm_type == 5))
		{
			for (int i = 0; i < MAX_ACTIVE_RUMBLES; i++)
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

		for (std::size_t i = 0; i < MAX_ACTIVE_RUMBLES; ++i)
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

				str << std::format("HIGH: {} / LOW: {} (Time left: {:.0f}%)", highValue * scale, lowValue * scale, duration01*100);

				color[0] = 0.f;
				color[2] = 1.f;
			}

			Game::R_AddCmdDrawText(str.str().data(), std::numeric_limits<int>::max(), font, 15.0f, (height * scale + 1) * (i + 1) + 4.0f, scale, scale, 0.0f, color, Game::ITEM_TEXTSTYLE_NORMAL);
		}
	}

	Rumble::Rumble()
	{
		if (ZoneBuilder::IsEnabled())
			return;

		// Rumble action
		Utils::Hook(0x4FB35D, CG_FireWeapon_FireSoundHk, HOOK_JUMP).install()->quick();
		// TODO: CG_BulletHitClientShield
		// TODO: CG_ExplosiveImpactOnShield
		// TODO: CG_ExplosiveSplashOnShield

		// TODO: CG_PlayRumbleOnEntity
		// TODO: CG_PlayRumbleOnPosition
		// TODO: CG_PlayRumbleLoopOnEntity
		// TODO: CG_PlayRumbleLoopOnPosition
		// TODO: CG_PlayRumbleOnClientSafe
		// TODO: CG_PlayRumbleOnClientScaledWithUpdate
		// TODO: CG_PlayLoopRumbleOnClient
		// TODO: CG_StopRumble
		// TODO: CG_StopAllRumbles

		// TODO: PlayNoteMappedRumbleAliases
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