#include <STDInclude.hpp>
#include "ClientCommand.hpp"

#include "GSC/Script.hpp"

using namespace Utils::String;

namespace Components
{
	std::unordered_map<std::string, std::function<void(Game::gentity_s*, const Command::ServerParams*)>> ClientCommand::HandlersSV;

	bool ClientCommand::CheatsEnabled;

	ClientCommand::CheatsScopedLock::CheatsScopedLock()
	{
		CheatsEnabled = true;
	}

	ClientCommand::CheatsScopedLock::~CheatsScopedLock()
	{
		CheatsEnabled = false;
	}

	bool ClientCommand::CheatsOk(const Game::gentity_s* ent)
	{
		const auto entNum = ent->s.number;

		if (!(*Game::g_cheats)->current.enabled && !CheatsEnabled)
		{
			Logger::Debug("Cheats are disabled!");
			Game::SV_GameSendServerCommand(entNum, Game::SV_CMD_CAN_IGNORE, VA("%c \"GAME_CHEATSNOTENABLED\"", 0x65));
			return false;
		}

		if (ent->health < 1)
		{
			Logger::Debug("Entity {} must be alive to use this command!", entNum);
			Game::SV_GameSendServerCommand(entNum, Game::SV_CMD_CAN_IGNORE, VA("%c \"GAME_MUSTBEALIVECOMMAND\"", 0x65));
			return false;
		}

		return true;
	}

	void ClientCommand::Add(const char* name, const std::function<void(Game::gentity_s*, const Command::ServerParams*)>& callback)
	{
		const auto command = Utils::String::ToLower(name);

		HandlersSV[command] = callback;
	}

	void ClientCommand::ClientCommandStub(const int clientNum)
	{
		const auto ent = &Game::g_entities[clientNum];

		if (!ent->client)
		{
			Logger::Debug("ClientCommand: client {} is not fully connected", clientNum);
			return;
		}

		Command::ServerParams params;
		const auto command = Utils::String::ToLower(params.get(0));

		if (const auto itr = HandlersSV.find(command); itr != HandlersSV.end())
		{
			itr->second(ent, &params);
			return;
		}

		Utils::Hook::Call<void(int)>(0x416790)(clientNum);
	}

	void ClientCommand::AddCheatCommands()
	{
		Add("noclip", Cmd_Noclip_f);
		Add("ufo", Cmd_UFO_f);

		Add("god", [](Game::gentity_s* ent, [[maybe_unused]] const Command::ServerParams* params)
		{
			if (!CheatsOk(ent))
				return;

			ent->flags ^= Game::FL_GODMODE;

			const auto entNum = ent->s.number;
			Logger::Debug("God toggled for entity {}", entNum);

			Game::SV_GameSendServerCommand(entNum, Game::SV_CMD_CAN_IGNORE, VA("%c \"%s\"", 0x65, (ent->flags & Game::FL_GODMODE) ? "GAME_GODMODE_ON" : "GAME_GODMODE_OFF"));
		});

		Add("demigod", [](Game::gentity_s* ent, [[maybe_unused]] const Command::ServerParams* params)
		{
			if (!CheatsOk(ent))
				return;

			ent->flags ^= Game::FL_DEMI_GODMODE;

			const auto entNum = ent->s.number;
			Logger::Debug("Demigod toggled for entity {}", entNum);

			Game::SV_GameSendServerCommand(entNum, Game::SV_CMD_CAN_IGNORE, VA("%c \"%s\"", 0x65, (ent->flags & Game::FL_DEMI_GODMODE) ? "GAME_DEMI_GODMODE_ON" : "GAME_DEMI_GODMODE_OFF"));
		});

		Add("notarget", [](Game::gentity_s* ent, [[maybe_unused]] const Command::ServerParams* params)
		{
			if (!CheatsOk(ent))
				return;

			ent->flags ^= Game::FL_NOTARGET;

			const auto entNum = ent->s.number;
			Logger::Debug("Notarget toggled for entity {}", entNum);

			Game::SV_GameSendServerCommand(entNum, Game::SV_CMD_CAN_IGNORE, VA("%c \"%s\"", 0x65, (ent->flags & Game::FL_NOTARGET) ? "GAME_NOTARGETON" : "GAME_NOTARGETOFF"));
		});

		Add("setviewpos", [](Game::gentity_s* ent, [[maybe_unused]] const Command::ServerParams* params)
		{
			assert(ent);

			if (!CheatsOk(ent))
				return;

			Game::vec3_t origin, angles{0.f, 0.f, 0.f};

			if (params->size() < 4 || params->size() > 6)
			{
				Game::SV_GameSendServerCommand(ent->s.number, Game::SV_CMD_CAN_IGNORE, VA("%c \"GAME_USAGE\x15: setviewpos x y z [yaw] [pitch]\n\"", 0x65));
				return;
			}

			for (auto i = 0; i < 3; i++)
			{
				origin[i] = std::strtof(params->get(i + 1), nullptr);
			}

			if (params->size() >= 5)
			{
				angles[1] = std::strtof(params->get(4), nullptr); // Yaw
			}

			if (params->size() == 6)
			{
				angles[0] = std::strtof(params->get(5), nullptr); // Pitch
			}

			Logger::Debug("Teleported entity {} to {:f} {:f} {:f}\nviewpos {:f} {:f}", ent->s.number,
				origin[0], origin[1], origin[2], angles[0], angles[2]);
			Game::TeleportPlayer(ent, origin, angles);
		});

		Add("give", [](Game::gentity_s* ent, [[maybe_unused]] const Command::ServerParams* params)
		{
			if (!CheatsOk(ent))
				return;

			if (params->size() < 2)
			{
				Game::SV_GameSendServerCommand(ent->s.number, Game::SV_CMD_CAN_IGNORE, VA("%c \"GAME_USAGE\x15: give <weapon name>\"", 0x65));
				return;
			}

			Game::level->initializing = 1;
			const auto* weaponName = params->get(1);
			Logger::Debug("Giving weapon {} to entity {}", weaponName, ent->s.number);
			const auto weaponIndex = Game::G_GetWeaponIndexForName(weaponName);

			if (weaponIndex == 0)
			{
				Game::level->initializing = 0;
				return;
			}

			if (Game::BG_GetWeaponDef(weaponIndex)->inventoryType == Game::weapInventoryType_t::WEAPINVENTORY_ALTMODE)
			{
				Logger::PrintError(Game::CON_CHANNEL_ERROR,
					"You can't directly spawn the altfire weapon '{}'. Spawn a weapon that has this altmode instead.\n", weaponName);
				Game::level->initializing = 0;
				return;
			}

			auto* weapEnt = Game::G_Spawn();
			std::memcpy(weapEnt->r.currentOrigin, ent->r.currentOrigin, sizeof(std::float_t[3]));
			Game::G_GetItemClassname(static_cast<int>(weaponIndex), weapEnt);
			Game::G_SpawnItem(weapEnt, static_cast<int>(weaponIndex));

			weapEnt->active = 1;
			const auto offHandClass = Game::BG_GetWeaponDef(weaponIndex)->offhandClass;
			if (offHandClass != Game::OFFHAND_CLASS_NONE)
			{
				auto* client = ent->client;
				if ((client->ps.weapCommon.offhandPrimary != offHandClass) && (client->ps.weapCommon.offhandSecondary != offHandClass))
				{
					switch (offHandClass)
					{
					case Game::OFFHAND_CLASS_FRAG_GRENADE:
					case Game::OFFHAND_CLASS_THROWINGKNIFE:
					case Game::OFFHAND_CLASS_OTHER:
						Logger::Debug("Setting offhandPrimary");
						client->ps.weapCommon.offhandPrimary = offHandClass;
						break;
					default:
						Logger::Debug("Setting offhandSecondary");
						client->ps.weapCommon.offhandSecondary = offHandClass;
						break;
					}
				}
			}

			Game::Touch_Item(weapEnt, ent, 0);
			weapEnt->active = 0;

			if (weapEnt->r.isInUse)
			{
				Logger::Debug("Freeing up entity {}", weapEnt->s.number);
				Game::G_FreeEntity(weapEnt);
			}

			Game::level->initializing = 0;

			for (std::size_t i = 0; i < std::extent_v<decltype(Game::playerState_s::weaponsEquipped)>; ++i)
			{
				const auto index = ent->client->ps.weaponsEquipped[i];
				if (index)
				{
					Game::Add_Ammo(ent, index, 0, 998, 1);
				}
			}
		});

		Add("kill", []([[maybe_unused]] Game::gentity_s* ent, [[maybe_unused]] const Command::ServerParams* params)
		{
			assert(ent->client);
			assert(ent->client->sess.connected != Game::CON_DISCONNECTED);

			if (ent->client->sess.sessionState != Game::SESS_STATE_PLAYING || !CheatsOk(ent))
				return;

			auto** bgs = Game::Sys::GetTls<Game::bgs_t*>(Game::Sys::TLS_OFFSET::LEVEL_BGS);

			assert(*bgs == nullptr);

			*bgs = Game::level_bgs;

			ent->flags &= ~(Game::FL_GODMODE | Game::FL_DEMI_GODMODE);
			ent->health = 0;
			ent->client->ps.stats[0] = 0;
			Game::player_die(ent, ent, ent, 100000, Game::MOD_SUICIDE, 0, nullptr, Game::HITLOC_NONE, 0);

			assert(*bgs == Game::level_bgs);

			*bgs = nullptr;
		});
	}

	void ClientCommand::AddDevelopmentCommands()
	{
		Add("dropallbots", []([[maybe_unused]] Game::gentity_s* ent, [[maybe_unused]] const Command::ServerParams* params)
		{
			Game::SV_DropAllBots();
		});

		Add("entitylist", []([[maybe_unused]] Game::gentity_s* ent, [[maybe_unused]] const Command::ServerParams* params)
		{
			Game::Svcmd_EntityList_f();
		});

		Add("printentities", []([[maybe_unused]] Game::gentity_s* ent, [[maybe_unused]] const Command::ServerParams* params)
		{
			Game::G_PrintEntities();
		});

		Add("entitycount", []([[maybe_unused]] Game::gentity_s* ent, [[maybe_unused]] const Command::ServerParams* params)
		{
			Logger::Print("Entity count = {}\n", Game::level->num_entities);
		});

		// Also known as: "vis"
		Add("visionsetnaked", []([[maybe_unused]] Game::gentity_s* ent, [[maybe_unused]] const Command::ServerParams* params)
		{
			if (params->size() < 2)
			{
				Logger::Print("USAGE: visionSetNaked <name> <duration>\n");
				return;
			}

			auto duration = 1000;
			if (params->size() > 2)
			{
				const auto input = std::strtof(params->get(2), nullptr);
				duration = static_cast<int>(std::floorf(input * 1000.0f + 0.5f));
			}

			assert(ent->client);

			constexpr auto visMode = Game::visionSetMode_t::VISIONSET_NORMAL;
			const auto* name = params->get(1);

			ent->client->visionDuration[visMode] = duration;
			strncpy_s(ent->client->visionName[visMode], sizeof(Game::gclient_s::visionName[0]) / sizeof(char), name, _TRUNCATE);

			Game::SV_GameSendServerCommand(ent->s.number, Game::SV_CMD_RELIABLE, VA("%c \"%s\" %i", Game::MY_CMDS[visMode], name, duration));
		});

		Add("visionsetnight", []([[maybe_unused]] Game::gentity_s* ent, [[maybe_unused]] const Command::ServerParams* params)
		{
			if (params->size() < 2)
			{
				Logger::Print("USAGE: visionSetNight <name> <duration>\n");
				return;
			}

			auto duration = 1000;
			if (params->size() > 2)
			{
				const auto input = std::strtof(params->get(2), nullptr);
				duration = static_cast<int>(std::floorf(input * 1000.0f + 0.5f));
			}

			assert(ent->client);

			constexpr auto visMode = Game::visionSetMode_t::VISIONSET_NIGHT;
			const auto* name = params->get(1);

			ent->client->visionDuration[visMode] = duration;
			strncpy_s(ent->client->visionName[visMode], sizeof(Game::gclient_s::visionName[0]) / sizeof(char), name, _TRUNCATE);

			Game::SV_GameSendServerCommand(ent->s.number, Game::SV_CMD_RELIABLE, VA("%c \"%s\" %i", Game::MY_CMDS[visMode], name, duration));
		});

		Add("g_testCmd", []([[maybe_unused]] Game::gentity_s* ent, [[maybe_unused]] const Command::ServerParams* params)
		{
			assert(ent);

			ent->client->ps.stunTime = 1000 + Game::level->time; // 1000 is the default test stun time
			Logger::Debug("playerState_s.stunTime is {}", ent->client->ps.stunTime);
		});

		Add("dumpEntInfo", []([[maybe_unused]] Game::gentity_s* ent, [[maybe_unused]] const Command::ServerParams* params)
		{
			G_DumpEntityDebugInfoToConsole(false);
		});

		Add("dumpEntInfoCSV", []([[maybe_unused]] Game::gentity_s* ent, [[maybe_unused]] const Command::ServerParams* params)
		{
			G_DumpEntityDebugInfoToCSV("");
		});
	}

	void ClientCommand::AddScriptFunctions()
	{
		GSC::Script::AddFunction("DropAllBots", [] // gsc: DropAllBots();
		{
			Game::SV_DropAllBots();
		});
	}

	void ClientCommand::AddScriptMethods()
	{
		GSC::Script::AddMethod("Noclip", [](const Game::scr_entref_t entref) // gsc: self Noclip();
		{
			auto* ent = GSC::Script::Scr_GetPlayerEntity(entref);

			CheatsScopedLock cheatsLock;
			Cmd_Noclip_f(ent, nullptr);
		});

		GSC::Script::AddMethod("Ufo", [](const Game::scr_entref_t entref) // gsc: self Ufo();
		{
			auto* ent = GSC::Script::Scr_GetPlayerEntity(entref);

			CheatsScopedLock cheatsLock;
			Cmd_UFO_f(ent, nullptr);
		});
	}

	const char* ClientCommand::EntInfoLine(const int entNum)
	{
		const auto* ent = &Game::g_entities[entNum];

		Game::XModel* model = nullptr;
		if (ent->model)
		{
			model = Game::G_GetModel(ent->model);
		}

		Game::vec3_t point, angles;

		point[0] = ent->r.currentOrigin[0] - (*Game::viewposNow)->current.vector[0];
		point[1] = ent->r.currentOrigin[1] - (*Game::viewposNow)->current.vector[1];
		point[2] = ent->r.currentOrigin[2] - (*Game::viewposNow)->current.vector[2];

		angles[0] = ent->r.currentAngles[0];
		angles[1] = ent->r.currentAngles[1];
		angles[2] = ent->r.currentAngles[2];

		const auto distance = std::sqrtf(point[0] * point[0] + point[1] * point[1]
			+ point[2] * point[2]);

		const auto* team = (ent->client) ? Game::CG_GetTeamName(ent->client->sess.cs.team) : "";

		const auto* scriptLinkName = (ent->script_linkName) ? Game::SL_ConvertToString(ent->script_linkName) : "";

		const auto* target = (ent->target) ? Game::SL_ConvertToString(ent->target) : "";

		const auto* targetName = (ent->targetname) ? Game::SL_ConvertToString(ent->targetname) : "";

		const auto* codeClassname = (ent->script_classname) ? Game::SL_ConvertToString(ent->script_classname) : "";

		const auto* classname = (ent->classname) ? Game::SL_ConvertToString(ent->classname) : "";

		const auto* eventType = (ent->s.eType < Game::ET_EVENTS) ? Game::G_GetEntityTypeName(ent) : "";

		// See description of the format string in the function G_DumpEntityDebugInfoToCSV
		// If empty it means the item does not exist in the current version of the game or it was not possible to reverse it
		return VA("%i,%s,%.0f,%s,%s,%s,%s,%s,%s,%s,%s,%s,%.0f %.0f %.0f,%.0f %.0f %.0f,%i\n",
			entNum, eventType, distance, classname, codeClassname, (model) ? model->name : "",
			targetName, target, "", scriptLinkName, team, "",
			point[0], point[1], point[2], angles[0], angles[1], angles[2], 0);
	}

	void ClientCommand::G_DumpEntityDebugInfoToConsole(bool logfileOnly)
	{
		const auto channel = logfileOnly ? Game::CON_CHANNEL_LOGFILEONLY : Game::CON_CHANNEL_SERVER;

		Logger::Print(channel, "=====================================================================================\n");
		Logger::Print(channel, "============(entity dump begin)\n");
		Logger::Print(channel,
			"Number,Type,Distance,Classname,Code Classname,Model,"
			"Targetname,Target,Script Noteworthy,Script Linkname,Team,ParentNum,Origin,Angles,SentToClients\n");

		for (auto i = 0; i < Game::MAX_GENTITIES; ++i)
		{
			if (&Game::g_entities[i] == nullptr)
			{
				continue;
			}

			const auto* line = EntInfoLine(i);
			assert(line);
			Logger::Print(channel, "%s", line);
		}

		Logger::Print(channel, "(end entity dump)============\n");
		Logger::Print(channel, "=====================================================================================\n");
	}

	void ClientCommand::G_DumpEntityDebugInfoToCSV(const char* filenameSuffix)
	{
		assert(filenameSuffix);

		const auto* fileName = VA("%s%s%s%s", "EntInfo", (*filenameSuffix) ? "_" : "", filenameSuffix, ".csv");
		Logger::Print(Game::CON_CHANNEL_SERVER, "Opening file \"{}\" for writing.\n", fileName);

		auto h = Game::FS_FOpenTextFileWrite(fileName);
		if (!h)
		{
			Logger::PrintError(Game::CON_CHANNEL_SERVER, "Couldn't open file \"{}\" for writing.\n", fileName);
			return;
		}

		Game::FS_Write("Number,Type,Distance,Classname,Code Classname,Model,Targetname,Target,Script Noteworthy,Script Linkname,Team,Paren"
			"tNum,Origin,Angles,SentToClients\n", 147, h);

		for (auto i = 0; i < Game::MAX_GENTITIES; ++i)
		{
			if (&Game::g_entities[i] == nullptr)
			{
				continue;
			}

			const auto* line = EntInfoLine(i);
			const auto lineLen = std::strlen(line);

			assert(line);
			assert(lineLen);

			Game::FS_Write(line, static_cast<int>(lineLen), h);
		}

		Game::FS_FCloseFile(h);
		Logger::Print(Game::CON_CHANNEL_SERVER, "Done writing file.\n");
	}

	void ClientCommand::Cmd_Noclip_f(Game::gentity_s* ent, [[maybe_unused]] const Command::ServerParams* params)
	{
		if (!CheatsOk(ent))
			return;

		ent->client->flags ^= Game::CF_BIT_NOCLIP;

		const auto entNum = ent->s.number;
		Logger::Debug("Noclip toggled for entity {}", entNum);

		Game::SV_GameSendServerCommand(entNum, Game::SV_CMD_CAN_IGNORE, VA("%c \"%s\"", 0x65, (ent->client->flags & Game::CF_BIT_NOCLIP) ? "GAME_NOCLIPON" : "GAME_NOCLIPOFF"));
	}

	void ClientCommand::Cmd_UFO_f(Game::gentity_s* ent, [[maybe_unused]] const Command::ServerParams* params)
	{
		if (!CheatsOk(ent))
			return;

		ent->client->flags ^= Game::CF_BIT_UFO;

		const auto entNum = ent->s.number;
		Logger::Debug("UFO toggled for entity {}", entNum);

		Game::SV_GameSendServerCommand(entNum, Game::SV_CMD_CAN_IGNORE, VA("%c \"%s\"", 0x65, (ent->client->flags & Game::CF_BIT_UFO) ? "GAME_UFOON" : "GAME_UFOOFF"));
	}

	ClientCommand::ClientCommand()
	{
		AssertOffset(Game::playerState_s, stats, 0x150);

		// Hook call to ClientCommand in SV_ExecuteClientCommand so we may add custom commands
		Utils::Hook(0x6259FA, ClientCommandStub, HOOK_CALL).install()->quick();

		CheatsEnabled = false;

		AddCheatCommands();
		AddScriptFunctions();
		AddScriptMethods();
#ifdef _DEBUG
		AddDevelopmentCommands();
#endif
	}
}
