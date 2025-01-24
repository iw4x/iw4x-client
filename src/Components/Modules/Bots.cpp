
#include "Bots.hpp"
#include "ClanTags.hpp"
#include "Events.hpp"

#include "GSC/Script.hpp"

// From Quake-III
#define ANGLE2SHORT(x) ((int)((x) * (USHRT_MAX + 1) / 360.0f) & USHRT_MAX)
#define SHORT2ANGLE(x) ((x)* (360.0f / (USHRT_MAX + 1)))

namespace Components
{
	constexpr std::size_t MAX_NAME_LENGTH = 16;

	Game::dvar_t** Bots::aim_automelee_range = reinterpret_cast<Game::dvar_t**>(0x7A2F98);
	Game::dvar_t** Bots::perk_extendedMeleeRange = reinterpret_cast<Game::dvar_t**>(0x7BDD60);

	const Game::dvar_t* Bots::sv_randomBotNames;
	const Game::dvar_t* Bots::sv_replaceBots;

	std::size_t Bots::BotDataIndex;

	std::vector<Bots::botData> Bots::RemoteBotNames;

	struct BotMovementInfo
	{
		std::int32_t buttons; // Actions
		std::int8_t forward;
		std::int8_t right;
		std::uint16_t weapon;
		std::uint16_t lastAltWeapon;
		std::uint8_t meleeDist;
		float meleeYaw;
		std::int8_t remoteAngles[2];
		float angles[3];
		bool active;
	};

	static BotMovementInfo g_botai[Game::MAX_CLIENTS];

	struct BotAction
	{
		std::string action;
		std::int32_t key;
	};

	static const BotAction BotActions[] =
	{
		{ "gostand", Game::CMD_BUTTON_UP },
		{ "gocrouch", Game::CMD_BUTTON_CROUCH },
		{ "goprone", Game::CMD_BUTTON_PRONE },
		{ "fire", Game::CMD_BUTTON_ATTACK },
		{ "melee", Game::CMD_BUTTON_MELEE },
		{ "frag", Game::CMD_BUTTON_FRAG },
		{ "smoke",  Game::CMD_BUTTON_OFFHAND_SECONDARY },
		{ "reload", Game::CMD_BUTTON_RELOAD },
		{ "sprint", Game::CMD_BUTTON_SPRINT },
		{ "leanleft", Game::CMD_BUTTON_LEAN_LEFT },
		{ "leanright", Game::CMD_BUTTON_LEAN_RIGHT },
		{ "ads", Game::CMD_BUTTON_ADS | Game::CMD_BUTTON_THROW },
		{ "holdbreath", Game::CMD_BUTTON_BREATH },
		{ "usereload", Game::CMD_BUTTON_USE_RELOAD },
		{ "activate", Game::CMD_BUTTON_ACTIVATE },
		{ "remote", Game::CMD_BUTTON_REMOTE },
	};

	void Bots::UpdateBotNames()
	{
		const auto masterPort = (*Game::com_masterPort)->current.integer;
		const auto* masterServerName = (*Game::com_masterServerName)->current.string;

		Network::Address master(Utils::String::VA("%s:%u", masterServerName, masterPort));

		Logger::Print("Getting bots...\n");
		Network::Send(master, "getbots");
	}

	std::vector<Bots::botData> Bots::LoadBotNames()
	{
		std::vector<botData> result;

		FileSystem::File bots("bots.txt");
		if (!bots.exists())
		{
			return result;
		}

		auto data = Utils::String::Split(bots.getBuffer(), '\n');

		for (auto& entry : data)
		{
			// Take into account CR line endings
			Utils::String::Replace(entry, "\r", "");
			// Remove whitespace
			Utils::String::Trim(entry);

			if (entry.empty())
			{
				continue;
			}

			std::string clanAbbrev;

			// Check if there is a clan tag
			if (const auto pos = entry.find(','); pos != std::string::npos)
			{
				// Only start copying over from non-null characters (otherwise it can be "<=")
				if ((pos + 1) < entry.size())
				{
					clanAbbrev = entry.substr(pos + 1, ClanTags::MAX_CLAN_NAME_LENGTH - 1);
				}

				entry = entry.substr(0, pos);
			}

			entry = entry.substr(0, MAX_NAME_LENGTH - 1);

			result.emplace_back(entry, clanAbbrev);
		}

		return result;
	}

	int Bots::BuildConnectString(char* buffer, const char* connectString, int num, int, int protocol, int checksum, int statVer, int stats, int port)
	{
		std::string botName;
		std::string clanName;

		static const auto botNames = []() -> std::vector<botData>
		{
			auto names = LoadBotNames();
			if (names.empty())
			{
				Logger::Print("bots.txt was empty. Using the names from the master server\n");
				names = RemoteBotNames;
			}

			if (sv_randomBotNames->current.enabled)
			{
				std::random_device rd;
				std::mt19937 gen(rd());
				std::ranges::shuffle(names, gen);
			}

			return names;
		}();

		if (!botNames.empty())
		{
			BotDataIndex %= botNames.size();
			const auto index = BotDataIndex++;
			botName = botNames[index].first;
			clanName = botNames[index].second;
		}
		else
		{
			botName = std::format("bot{}", num);
			clanName = "BOT"s;
		}

		return _snprintf_s(buffer, 0x400, _TRUNCATE, connectString, num, botName.data(), clanName.data(), protocol, checksum, statVer, stats, port);
	}

	void Bots::Spawn(unsigned int count)
	{
		for (std::size_t i = 0; i < count; ++i)
		{
			Scheduler::Once([]
			{
				auto* ent = Game::SV_AddTestClient();
				if (!ent)
				{
					return;
				}

				Scheduler::Once([ent]
				{
					Game::Scr_AddString("autoassign");
					Game::Scr_AddString("team_marinesopfor");
					Game::Scr_Notify(ent, static_cast<std::uint16_t>(Game::SL_GetString("menuresponse", 0)), 2);

					Scheduler::Once([ent]
					{
						Game::Scr_AddString(Utils::String::Format("class{}", std::rand() % 5));
						Game::Scr_AddString("changeclass");
						Game::Scr_Notify(ent, static_cast<std::uint16_t>(Game::SL_GetString("menuresponse", 0)), 2);
					}, Scheduler::Pipeline::SERVER, 1s);

				}, Scheduler::Pipeline::SERVER, 1s);

			}, Scheduler::Pipeline::SERVER, 500ms * (i + 1));
		}
	}

	void Bots::GScr_isTestClient(const Game::scr_entref_t entref)
	{
		const auto* ent = Game::GetEntity(entref);
		if (!ent->client)
		{
			Game::Scr_Error("isTestClient: entity must be a player entity");
			return;
		}

		Game::Scr_AddBool(Game::SV_IsTestClient(ent->s.number) != 0);
	}

	bool Bots::BG_HasPerk(const unsigned int* perks, const unsigned int perkIndex)
	{
		assert(perks);
		AssertIn(perkIndex, Game::PERK_COUNT);
	
		const std::size_t arrayIndex = perkIndex >> 5;
		AssertIn(arrayIndex, Game::PERK_ARRAY_COUNT);
	
		constexpr std::size_t BIT_MASK_SIZE = 31; // 0x1F
		const auto bitPos = perkIndex & BIT_MASK_SIZE;
		const auto bitMask = 1 << bitPos;
	
		return perks[arrayIndex] & bitMask;
	}

	void Bots::AddScriptMethods()
	{
		GSC::Script::AddMethMultiple(GScr_isTestClient, false, {"IsTestClient", "IsBot"}); // Usage: self IsTestClient();

		GSC::Script::AddMethod("BotStop", [](const Game::scr_entref_t entref) // Usage: <bot> BotStop();
		{
			const auto* ent = GSC::Script::Scr_GetPlayerEntity(entref);
			if (!Game::SV_IsTestClient(ent->s.number))
			{
				Game::Scr_Error("BotStop: Can only call on a bot!");
				return;
			}

			ZeroMemory(&g_botai[entref.entnum], sizeof(BotMovementInfo));
			g_botai[entref.entnum].weapon = static_cast<std::uint16_t>(ent->client->ps.weapCommon.weapon);
			g_botai[entref.entnum].angles[0] = ent->client->ps.viewangles[0];
			g_botai[entref.entnum].angles[1] = ent->client->ps.viewangles[1];
			g_botai[entref.entnum].angles[2] = ent->client->ps.viewangles[2];
			g_botai[entref.entnum].active = true;
		});

		GSC::Script::AddMethod("BotWeapon", [](const Game::scr_entref_t entref) // Usage: <bot> BotWeapon(<str>);
		{
			const auto* ent = GSC::Script::Scr_GetPlayerEntity(entref);
			if (!Game::SV_IsTestClient(ent->s.number))
			{
				Game::Scr_Error("BotWeapon: Can only call on a bot!");
				return;
			}

			const auto* weapon = Game::Scr_GetString(0);
			if (!weapon || !*weapon)
			{
				g_botai[entref.entnum].weapon = 1;
				return;
			}

			const auto weapId = Game::G_GetWeaponIndexForName(weapon);
			g_botai[entref.entnum].weapon = static_cast<uint16_t>(weapId);
			g_botai[entref.entnum].active = true;
		});

		GSC::Script::AddMethod("BotAction", [](const Game::scr_entref_t entref) // Usage: <bot> BotAction(<str action>);
		{
			const auto* ent = GSC::Script::Scr_GetPlayerEntity(entref);
			if (!Game::SV_IsTestClient(ent->s.number))
			{
				Game::Scr_Error("BotAction: Can only call on a bot!");
				return;
			}

			const auto* action = Game::Scr_GetString(0);
			if (!action)
			{
				Game::Scr_ParamError(0, "BotAction: Illegal parameter!");
				return;
			}

			if (action[0] != '+' && action[0] != '-')
			{
				Game::Scr_ParamError(0, "BotAction: Sign for action must be '+' or '-'");
				return;
			}

			for (std::size_t i = 0; i < std::extent_v<decltype(BotActions)>; ++i)
			{
				if (Utils::String::ToLower(&action[1]) != BotActions[i].action)
					continue;

				if (action[0] == '+')
					g_botai[entref.entnum].buttons |= BotActions[i].key;
				else
					g_botai[entref.entnum].buttons &= ~BotActions[i].key;

				g_botai[entref.entnum].active = true;
				return;
			}

			Game::Scr_ParamError(0, "BotAction: Unknown action");
		});

		GSC::Script::AddMethod("BotMovement", [](const Game::scr_entref_t entref) // Usage: <bot> BotMovement(<int>, <int>);
		{
			const auto* ent = GSC::Script::Scr_GetPlayerEntity(entref);
			if (!Game::SV_IsTestClient(ent->s.number))
			{
				Game::Scr_Error("BotMovement: Can only call on a bot!");
				return;
			}

			const auto forwardInt = std::clamp<int>(Game::Scr_GetInt(0), std::numeric_limits<char>::min(), std::numeric_limits<char>::max());
			const auto rightInt = std::clamp<int>(Game::Scr_GetInt(1), std::numeric_limits<char>::min(), std::numeric_limits<char>::max());

			g_botai[entref.entnum].forward = static_cast<int8_t>(forwardInt);
			g_botai[entref.entnum].right = static_cast<int8_t>(rightInt);
			g_botai[entref.entnum].active = true;
		});

		GSC::Script::AddMethod("BotMeleeParams", [](const Game::scr_entref_t entref) // Usage: <bot> BotMeleeParams(<float>, <float>);
		{
			const auto* ent = GSC::Script::Scr_GetPlayerEntity(entref);
			if (!Game::SV_IsTestClient(ent->s.number))
			{
				Game::Scr_Error("BotMeleeParams: Can only call on a bot!");
				return;
			}

			if (!(*perk_extendedMeleeRange))
			{
				Game::Scr_Error("BotMeleeParams: perk_extendedMeleeRange is not registered!");
				return;
			}
			
			if (!(*aim_automelee_range))
			{
				Game::Scr_Error("BotMeleeParams: aim_automelee_range is not registered!");
				return;
			}
			
			const auto yaw = Game::Scr_GetFloat(0);
			const auto dist = static_cast<int>(Game::Scr_GetFloat(1));
			constexpr auto minDist = static_cast<int>(std::numeric_limits<unsigned char>::min());
			const bool isExtendedMelee = BG_HasPerk(ent->client->ps.perks, Game::PERK_EXTENDEDMELEE);
			const auto maxDist = isExtendedMelee
				? static_cast<int>((*perk_extendedMeleeRange)->current.value)
				: static_cast<int>((*aim_automelee_range)->current.value);
			const auto meleeDist = std::clamp<int>(dist, minDist, maxDist);
			
			g_botai[entref.entnum].meleeYaw = yaw;
			g_botai[entref.entnum].meleeDist = static_cast<int8_t>(meleeDist);
			g_botai[entref.entnum].active = true;
		});

		GSC::Script::AddMethod("BotRemoteAngles", [](const Game::scr_entref_t entref) // Usage: <bot> BotRemoteAngles(<int>, <int>);
		{
			const auto* ent = GSC::Script::Scr_GetPlayerEntity(entref);
			if (!Game::SV_IsTestClient(ent->s.number))
			{
				Game::Scr_Error("BotRemoteAngles: Can only call on a bot!");
				return;
			}

			const auto pitch = std::clamp<int>(Game::Scr_GetInt(0), std::numeric_limits<char>::min(), std::numeric_limits<char>::max());
			const auto yaw = std::clamp<int>(Game::Scr_GetInt(1), std::numeric_limits<char>::min(), std::numeric_limits<char>::max());

			g_botai[entref.entnum].remoteAngles[0] = static_cast<int8_t>(pitch);
			g_botai[entref.entnum].remoteAngles[1] = static_cast<int8_t>(yaw);
			g_botai[entref.entnum].active = true;
		});

		GSC::Script::AddMethod("BotAngles", [](const Game::scr_entref_t entref) // Usage: <bot> BotAngles(<float>, <float>, <float>);
		{
			const auto* ent = GSC::Script::Scr_GetPlayerEntity(entref);
			if (!Game::SV_IsTestClient(ent->s.number))
			{
				Game::Scr_Error("BotAngles: Can only call on a bot!");
				return;
			}

			const auto pitch = Game::Scr_GetFloat(0);
			const auto yaw = Game::Scr_GetFloat(1);
			const auto roll = Game::Scr_GetFloat(2);

			g_botai[entref.entnum].angles[0] = pitch;
			g_botai[entref.entnum].angles[1] = yaw;
			g_botai[entref.entnum].angles[2] = roll;

			g_botai[entref.entnum].active = true;
		});
	}

	void Bots::BotAiAction(Game::client_s* cl)
	{
		if (!cl->gentity)
		{
			return;
		}

		auto clientNum = cl - Game::svs_clients;

		// Keep test client functionality
		if (!g_botai[clientNum].active)
		{
			Game::SV_BotUserMove(cl);
			return;
		}

		Game::usercmd_s userCmd;
		ZeroMemory(&userCmd, sizeof(Game::usercmd_s));

		userCmd.serverTime = *Game::svs_time;

		userCmd.buttons = g_botai[clientNum].buttons;
		userCmd.forwardmove = g_botai[clientNum].forward;
		userCmd.rightmove = g_botai[clientNum].right;
		userCmd.weapon = g_botai[clientNum].weapon;
		userCmd.primaryWeaponForAltMode = g_botai[clientNum].lastAltWeapon;
		userCmd.meleeChargeYaw = g_botai[clientNum].meleeYaw;
		userCmd.meleeChargeDist = g_botai[clientNum].meleeDist;
		userCmd.remoteControlAngles[0] = g_botai[clientNum].remoteAngles[0];
		userCmd.remoteControlAngles[1] = g_botai[clientNum].remoteAngles[1];

		userCmd.angles[0] = ANGLE2SHORT(g_botai[clientNum].angles[0] - cl->gentity->client->ps.delta_angles[0]);
		userCmd.angles[1] = ANGLE2SHORT(g_botai[clientNum].angles[1] - cl->gentity->client->ps.delta_angles[1]);
		userCmd.angles[2] = ANGLE2SHORT(g_botai[clientNum].angles[2] - cl->gentity->client->ps.delta_angles[2]);

		Game::SV_ClientThink(cl, &userCmd);
	}

	__declspec(naked) void Bots::SV_BotUserMove_Hk()
	{
		__asm
		{
			pushad

			push edi
			call BotAiAction
			add esp, 4

			popad
			ret
		}
	}

	void Bots::G_SelectWeaponIndex(int clientNum, unsigned int iWeaponIndex)
	{
		if (g_botai[clientNum].active)
		{
			g_botai[clientNum].weapon = static_cast<uint16_t>(iWeaponIndex);
			g_botai[clientNum].lastAltWeapon = 0;

			auto* def = Game::BG_GetWeaponCompleteDef(iWeaponIndex);

			if (def && def->weapDef->inventoryType == Game::WEAPINVENTORY_ALTMODE)
			{
				auto* ps = &Game::g_entities[clientNum].client->ps;
				auto numWeaps = Game::BG_GetNumWeapons();

				for (auto i = 1u; i < numWeaps; i++)
				{
					if (!Game::BG_PlayerHasWeapon(ps, i))
					{
						continue;
					}

					auto* thisDef = Game::BG_GetWeaponCompleteDef(i);

					if (!thisDef || thisDef->altWeaponIndex != iWeaponIndex)
					{
						continue;
					}

					g_botai[clientNum].lastAltWeapon = static_cast<uint16_t>(i);
					break;
				}
			}
		}
	}

	__declspec(naked) void Bots::G_SelectWeaponIndex_Hk()
	{
		__asm
		{
			pushad

			push [esp + 0x20 + 0x8]
			push [esp + 0x20 + 0x8]
			call G_SelectWeaponIndex
			add esp, 0x8

			popad

			// Code skipped by hook
			mov eax, [esp + 0x8]
			push eax

			push 0x441B85
			retn
		}
	}

	int Bots::SV_GetClientPing_Hk(const int clientNum)
	{
		AssertIn(clientNum, Game::MAX_CLIENTS);

		if (Game::SV_IsTestClient(clientNum))
		{
			return -1;
		}

		return Game::svs_clients[clientNum].ping;
	}

	bool Bots::IsFull()
	{
		auto i = 0;
		while (i < *Game::svs_clientCount)
		{
			if (Game::svs_clients[i].header.state == Game::CS_FREE)
			{
				// Free slot was found
				break;
			}

			++i;
		}

		return i == *Game::svs_clientCount;
	}

	void Bots::SV_DirectConnect_Full_Check()
	{
		if (!sv_replaceBots->current.enabled || !IsFull())
		{
			return;
		}

		for (auto i = 0; i < (*Game::sv_maxclients)->current.integer; ++i)
		{
			auto* cl = &Game::svs_clients[i];
			if (cl->bIsTestClient)
			{
				Game::SV_DropClient(cl, "EXE_DISCONNECTED", false);
				cl->header.state = Game::CS_FREE;
				return;
			}
		}
	}

	void Bots::CleanBotArray()
	{
		ZeroMemory(&g_botai, sizeof(g_botai));
		for (std::size_t i = 0; i < std::extent_v<decltype(g_botai)>; ++i)
		{
			g_botai[i].weapon = 1; // Prevent the bots from defaulting to the 'none' weapon
		}
	}

	void Bots::AddServerCommands()
	{
		Command::AddSV("spawnBot", [](const Command::Params* params)
		{
			if (!Dedicated::IsRunning())
			{
				Logger::Print("Server is not running.\n");
				return;
			}

			if (IsFull())
			{
				Logger::Warning(Game::CON_CHANNEL_DONT_FILTER, "Server is full.\n");
				return;
			}

			std::size_t count = 1;

			if (params->size() > 1)
			{
				if (params->get(1) == "all"s)
				{
					count = Game::MAX_CLIENTS;
				}
				else
				{
					char* end;
					const auto* input = params->get(1);
					count = std::strtoul(input, &end, 10);

					if (input == end)
					{
						Logger::Warning(Game::CON_CHANNEL_DONT_FILTER, "{} is not a valid input\nUsage: {} optional <number of bots> or optional <\"all\">\n", input, params->get(0));
						return;
					}
				}
			}

			count = std::clamp<std::size_t>(count, 1, Game::MAX_CLIENTS);

			Logger::Print("Spawning {} {}\n", count, (count == 1 ? "bot" : "bots"));

			Spawn(count);
		});
	}

	bool Bots::Player_UpdateActivate_stub(int)
	{
		return false;
	}

	Bots::Bots()
	{
		AssertOffset(Game::client_s, bIsTestClient, 0x41AF0);
		AssertOffset(Game::client_s, ping, 0x212C8);
		AssertOffset(Game::client_s, gentity, 0x212A0);

		// Replace connect string
		Utils::Hook::Set<const char*>(0x48ADA6, "connect bot%d \"\\cg_predictItems\\1\\cl_anonymous\\0\\color\\4\\head\\default\\model\\multi\\snaps\\20\\rate\\5000\\name\\%s\\clanAbbrev\\%s\\protocol\\%d\\checksum\\%d\\statver\\%d %u\\qport\\%d\"");

		// Intercept sprintf for the connect string
		Utils::Hook(0x48ADAB, BuildConnectString, HOOK_CALL).install()->quick();

		Utils::Hook(0x627021, SV_BotUserMove_Hk, HOOK_CALL).install()->quick();
		Utils::Hook(0x627241, SV_BotUserMove_Hk, HOOK_CALL).install()->quick();

		Utils::Hook(0x441B80, G_SelectWeaponIndex_Hk, HOOK_JUMP).install()->quick();

		// fix bots using objects (SV_IsClientBot)
		Utils::Hook(0x4D79C5, Player_UpdateActivate_stub, HOOK_CALL).install()->quick();

		Utils::Hook(0x459654, SV_GetClientPing_Hk, HOOK_CALL).install()->quick();

		sv_randomBotNames = Game::Dvar_RegisterBool("sv_randomBotNames", false, Game::DVAR_NONE, "Randomize the bots' names");
		sv_replaceBots = Game::Dvar_RegisterBool("sv_replaceBots", false, Game::DVAR_NONE, "Test clients will be replaced by connecting players when the server is full.");
		
		Scheduler::OnGameInitialized(UpdateBotNames, Scheduler::Pipeline::MAIN);

		Network::OnClientPacket("getbotsResponse", [](const Network::Address& address, const std::string& data)
		{
			const auto masterPort = (*Game::com_masterPort)->current.integer;
			const auto* masterServerName = (*Game::com_masterServerName)->current.string;

			Network::Address master(Utils::String::VA("%s:%u", masterServerName, masterPort));
			if (master == address)
			{
				auto botNames = Utils::String::Split(data, '\n');
				Logger::Print("Got {} names from the master server\n", botNames.size());

				for (const auto& entry : botNames)
				{
					RemoteBotNames.emplace_back(entry, "BOT");
				}
			}
		});

		// Reset BotMovementInfo.active when client is dropped
		Events::OnClientDisconnect([](const int clientNum) -> void
		{
			g_botai[clientNum].active = false;
		});

		Events::OnSVInit(AddServerCommands);

		CleanBotArray();

		AddScriptMethods();

		// In case a loaded mod didn't call "BotStop" before the VM shutdown
		Events::OnVMShutdown(CleanBotArray);
	}
}
