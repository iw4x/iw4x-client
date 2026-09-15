#include "ClientCommand.hpp"
#include "Events.hpp"
#include "ServerCommands.hpp"

#include "ModelCache.hpp"

#include "GSC/Script.hpp"

using namespace Utils::String;

namespace Components
{
	std::unordered_map<std::string, std::function<void(Game::gentity_s*, const Command::ServerParams*)>> ClientCommand::HandlersSV;

	bool ClientCommand::CheatsEnabled;

	std::array<bool, Game::MAX_CLIENTS> ClientCommand::GiveAllClients;
	std::unordered_map<std::uintptr_t, ClientCommand::GiveAllSite> ClientCommand::GiveAllSites;
	Game::dvar_t ClientCommand::GiveAllDvar;

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

			auto* target = params->size() > 2 ? GetPlayerEntity(params->get(1)) : ent;

			if (params->size() < 2 || !target)
			{
				Game::SV_GameSendServerCommand(ent->s.number, Game::SV_CMD_CAN_IGNORE, VA("%c \"GAME_USAGE\x15: give [entity] <weapon name|all|ammo>\"", 0x65));
				return;
			}

			if (target != ent && !CheatsOk(target))
				return;

			Give(target, params->get(params->size() > 2 ? 2 : 1));
		});

		Add("take", [](Game::gentity_s* ent, [[maybe_unused]] const Command::ServerParams* params)
		{
			if (!CheatsOk(ent))
				return;

			auto* target = params->size() > 2 ? GetPlayerEntity(params->get(1)) : ent;

			if (params->size() < 2 || !target)
			{
				Game::SV_GameSendServerCommand(ent->s.number, Game::SV_CMD_CAN_IGNORE, VA("%c \"GAME_USAGE\x15: take [entity] <weapon name|all>\"", 0x65));
				return;
			}

			if (target != ent && !CheatsOk(target))
				return;

			Take(target, params->get(params->size() > 2 ? 2 : 1));
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
			if (Components::ModelCache::modelsHaveBeenReallocated)
			{
				model = Components::ModelCache::cached_models_reallocated[ent->model];
			}
			else
			{
				model = Game::G_GetModel(ent->model);
			}
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

	void ClientCommand::AddServerCommands()
	{
		Command::AddSV("give", [](const Command::Params* params)
		{
			if (!Dedicated::IsRunning())
			{
				Logger::Print("Server is not running.\n");
				return;
			}

			if (params->size() < 3)
			{
				Logger::Print("{} <entity> <weapon name|all|ammo>\n", params->get(0));
				return;
			}

			auto* ent = GetPlayerEntity(params->get(1));
			if (!ent)
			{
				Logger::Print("Bad entity: {}\n", params->get(1));
				return;
			}

			if (!CheatsOk(ent))
				return;

			Give(ent, params->get(2));
		});

		Command::AddSV("take", [](const Command::Params* params)
		{
			if (!Dedicated::IsRunning())
			{
				Logger::Print("Server is not running.\n");
				return;
			}

			if (params->size() < 3)
			{
				Logger::Print("{} <entity> <weapon name|all>\n", params->get(0));
				return;
			}

			auto* ent = GetPlayerEntity(params->get(1));
			if (!ent)
			{
				Logger::Print("Bad entity: {}\n", params->get(1));
				return;
			}

			if (!CheatsOk(ent))
				return;

			Take(ent, params->get(2));
		});
	}

	Game::gentity_s* ClientCommand::GetPlayerEntity(const char* input)
	{
		char* end;
		const auto entNum = std::strtoul(input, &end, 10);

		if (input == end || *end != '\0' || entNum >= Game::MAX_CLIENTS)
		{
			return nullptr;
		}

		auto* ent = &Game::g_entities[entNum];
		if (!ent->client || ent->client->sess.connected == Game::CON_DISCONNECTED)
		{
			return nullptr;
		}

		return ent;
	}

	void ClientCommand::Give(Game::gentity_s* ent, const char* weaponName)
	{
		if (ToLower(weaponName) == "ammo")
		{
			Logger::Debug("Giving max ammo to entity {}", ent->s.number);
			GiveMaxAmmo(ent);
			return;
		}

		if (ToLower(weaponName) == "all")
		{
			Logger::Debug("Giving all weapons to entity {}", ent->s.number);
			GiveAllWeapons(ent);
			return;
		}

		Game::level->initializing = 1;
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
		SetOffhandClass(ent, weaponIndex);

		Game::Touch_Item(weapEnt, ent, 0);
		weapEnt->active = 0;

		if (weapEnt->r.isInUse)
		{
			Logger::Debug("Freeing up entity {}", weapEnt->s.number);
			Game::G_FreeEntity(weapEnt);
		}

		Game::level->initializing = 0;

		GiveMaxAmmo(ent);
	}

	void ClientCommand::Take(Game::gentity_s* ent, const char* weaponName)
	{
		auto* client = ent->client;

		if (ToLower(weaponName) == "all")
		{
			Logger::Debug("Taking all weapons from entity {}", ent->s.number);

			SetGiveAll(ent, false);

			client->ps.weapCommon.weapon = 0;

			for (std::size_t i = 0; i < std::extent_v<decltype(Game::playerState_s::weaponsEquipped)>; ++i)
			{
				const auto index = client->ps.weaponsEquipped[i];
				if (index && Game::BG_GetWeaponDef(index)->inventoryType != Game::weapInventoryType_t::WEAPINVENTORY_ALTMODE)
				{
					Game::BG_TakePlayerWeapon(&client->ps, index);
				}
			}

			return;
		}

		const auto weaponIndex = Game::G_GetWeaponIndexForName(weaponName);
		if (weaponIndex == 0)
		{
			return;
		}

		Logger::Debug("Taking weapon {} from entity {}", weaponName, ent->s.number);
		Game::BG_TakePlayerWeapon(&client->ps, weaponIndex);
	}

	void ClientCommand::GiveMaxAmmo(Game::gentity_s* ent)
	{
		auto* client = ent->client;

		if (HasGiveAll(&client->ps))
		{
			GiveAllAmmo(ent, true);
			return;
		}

		for (std::size_t i = 0; i < std::extent_v<decltype(Game::playerState_s::weaponsEquipped)>; ++i)
		{
			const auto index = client->ps.weaponsEquipped[i];
			if (index)
			{
				Game::Add_Ammo(ent, index, client->ps.weapEquippedData[i].weaponModel, 998, 1);
			}
		}
	}

	void ClientCommand::GiveAllWeapons(Game::gentity_s* ent)
	{
		SetGiveAll(ent, true);
		GiveAllAmmo(ent, true);
	}

	void ClientCommand::SetOffhandClass(Game::gentity_s* ent, const unsigned int weaponIndex)
	{
		const auto offHandClass = Game::BG_GetWeaponDef(weaponIndex)->offhandClass;
		if (offHandClass == Game::OFFHAND_CLASS_NONE)
		{
			return;
		}

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

	bool ClientCommand::HasGiveAll(const Game::playerState_s* ps)
	{
		if (*Game::g_giveAll && (*Game::g_giveAll)->current.enabled)
		{
			return true;
		}

		if (!ps)
		{
			return false;
		}

		const auto clientNum = static_cast<std::size_t>(ps->clientNum);
		return clientNum < GiveAllClients.size() && GiveAllClients[clientNum];
	}

	void ClientCommand::SetGiveAll(Game::gentity_s* ent, const bool enabled)
	{
		const auto clientNum = ent->s.number;
		GiveAllClients[clientNum] = enabled;

		Game::SV_GameSendServerCommand(clientNum, Game::SV_CMD_RELIABLE, VA("%c %i %i", 23, clientNum, enabled ? 1 : 0));
	}

	void ClientCommand::GiveAll_Hk(std::uintptr_t* frame)
	{
		const auto itr = GiveAllSites.find(frame[8] - 5);
		assert(itr != GiveAllSites.end());

		const auto& site = itr->second;
		const Game::playerState_s* ps = nullptr;

		switch (site.source)
		{
		case GiveAllSource::StackArgument:
			ps = *reinterpret_cast<Game::playerState_s**>(reinterpret_cast<std::uintptr_t>(frame + 9) + site.location);
			break;
		case GiveAllSource::PlayerState:
			ps = reinterpret_cast<Game::playerState_s*>(frame[site.location]);
			break;
		case GiveAllSource::Entity:
			if (const auto* ent = reinterpret_cast<Game::gentity_s*>(frame[site.location]); ent && ent->client)
			{
				ps = &ent->client->ps;
			}
			break;
		case GiveAllSource::PredictedPlayerState:
			ps = &Game::cgArray->predictedPlayerState;
			break;
		}

		frame[site.target] = HasGiveAll(ps) ? reinterpret_cast<std::uintptr_t>(&GiveAllDvar) : reinterpret_cast<std::uintptr_t>(*Game::g_giveAll);
	}

	__declspec(naked) void ClientCommand::GiveAll_Stub()
	{
		__asm
		{
			pushad
			push esp
			call GiveAll_Hk
			add esp, 4
			popad
			ret
		}
	}

	void ClientCommand::PatchGiveAll()
	{
		Utils::Hook::Set<std::uint32_t>(0x5E4492, Game::DVAR_CHEAT | Game::DVAR_INTERNAL);

		GiveAllDvar.name = "g_giveAll";
		GiveAllDvar.type = Game::DVAR_TYPE_BOOL;
		GiveAllDvar.current.enabled = true;

		// The story here is a little involved. So let's first explain what we
		// are trying to achieve and then how all this machine state business
		// comes into it.
		//
		// The stock game has g_giveAll as a regular global dvar. Weapon code
		// reads it in a number of places and, if it is enabled, behaves as if
		// the player has access to every weapon. This is fine for the original
		// use of the dvar where give-all really is global.
		//
		// For our `give all` command we want slightly different semantics. The
		// command applies to one client, so setting the real g_giveAll dvar
		// would make every player get the same behavior.
		//
		// We could, of course, patch all the weapon code that depends on
		// g_giveAll and teach each place about GiveAllClients. There are quite
		// a few such places and most of them already do exactly what we need
		// once they see a true g_giveAll dvar. So let's leave that code alone
		// and substitute the dvar pointer that it sees.
		//
		// GiveAllDvar is this substitute. It looks like g_giveAll to the engine
		// and has its boolean value permanently set to true. At each site below
		// we decide which pointer the original code should see:
		//
		//   &GiveAllDvar
		//
		// if give-all is enabled for the player involved in the current weapon
		// operation, or:
		//
		//   *Game::g_giveAll
		//
		// in every other case.
		//
		// Note that this preserves the stock global g_giveAll behavior. If the
		// real dvar is enabled, HasGiveAll() returns true independently of the
		// client state and these sites see a true dvar just as they did before.
		//
		// Now, the slightly awkward part is figuring out which player a given
		// g_giveAll read belongs to.
		//
		// These reads are spread across different engine functions and there is
		// no useful convention such as "the player is always in EDI". At one
		// site we have playerState_s* in ESI. Somewhere else EDI contains
		// gentity_s*. At another site the pointer we need lives on the stack.
		// The client prediction paths already have their player in
		// cgArray->predictedPlayerState.
		//
		// So each GiveAllSite records two things we recovered from the
		// disassembly:
		//
		//   1. Where to find the player associated with this g_giveAll read.
		//
		//   2. Which register the original instruction used to receive the
		//      g_giveAll pointer.
		//
		// The second part is easy to miss. We are replacing an instruction, so
		// finding the player and deciding which dvar to use is only part of the
		// job. When control returns to the engine we have to leave the machine
		// state in the form that the replaced instruction would have produced,
		// with the possible substitution of GiveAllDvar for g_giveAll.
		//
		// For example, suppose the original instruction was effectively:
		//
		//   EAX = g_giveAll;
		//
		// Then our hook has to return with EAX containing either
		// *Game::g_giveAll or &GiveAllDvar. Putting the right pointer in ECX
		// does us no good since the next instruction expects it in EAX.
		//
		// This is what the final REG_* value in every entry describes.
		//
		// Take this entry:
		//
		//   { 0x43ACD4, 5, GiveAllSource::PlayerState, REG_ESI, REG_EAX }
		//
		// Reading it from left to right:
		//
		//   0x43ACD4
		//
		// is the address of the engine instruction that used to read
		// g_giveAll.
		//
		//   5
		//
		// is the size of that instruction in bytes.
		//
		//   GiveAllSource::PlayerState
		//
		// says that `location` identifies a saved register containing
		// playerState_s*.
		//
		//   REG_ESI
		//
		// says that ESI contains playerState_s* at this exact point in the
		// function.
		//
		//   REG_EAX
		//
		// says that the original instruction placed the g_giveAll pointer in
		// EAX.
		//
		// Or, written approximately as C++:
		//
		//   player = reinterpret_cast<playerState_s*>(ESI);
		//
		//   EAX = HasGiveAll(player)
		//       ? &GiveAllDvar
		//       : *Game::g_giveAll;
		//
		// Of course, we cannot put all of that at 0x43ACD4. We have five bytes
		// there and we need to preserve the register state around the original
		// instruction. This is where GiveAll_Stub and PUSHAD come in.
		//
		// Each site is replaced with a near CALL to GiveAll_Stub. On x86
		// this CALL takes five bytes. GiveAll_Stub then does:
		//
		//   pushad
		//   push esp
		//   call GiveAll_Hk
		//   add  esp, 4
		//   popad
		//   ret
		//
		// PUSHAD saves all of the general-purpose registers for us. This gives
		// GiveAll_Hk() a snapshot of the register state at the point where the
		// engine reached the patched instruction.
		//
		// There is one x86 detail here that is rather easy to get backwards
		// from the C++ side.
		//
		// PUSHAD pushes:
		//
		//   EAX
		//   ECX
		//   EDX
		//   EBX
		//   original ESP
		//   EBP
		//   ESI
		//   EDI
		//
		// The stack grows downwards. So once PUSHAD has finished, ESP points at
		// the last value pushed, EDI. When GiveAll_Hk() receives this ESP as a
		// uintptr_t array, the layout looks like this:
		//
		//   frame[0] = EDI
		//   frame[1] = ESI
		//   frame[2] = EBP
		//   frame[3] = original ESP saved by PUSHAD
		//   frame[4] = EBX
		//   frame[5] = EDX
		//   frame[6] = ECX
		//   frame[7] = EAX
		//
		// Immediately above the PUSHAD frame we have:
		//
		//   frame[8] = return address pushed by our hook CALL
		//
		// and after that we are back at the stack that existed at the original
		// patch site:
		//
		//   frame[9] = value at [ESP + 0] before our CALL
		//   ...
		//
		// Note that REG_EAX and friends are indices into this PUSHAD frame. They
		// are not the register encodings used by x86 instructions. REG_EAX, for
		// example, is the index for frame[7].
		//
		// With the current stub we have:
		//
		//   REG_EDI -> frame[0]
		//   REG_ESI -> frame[1]
		//   REG_EBP -> frame[2]
		//   REG_ESP -> frame[3]
		//   REG_EBX -> frame[4]
		//   REG_EDX -> frame[5]
		//   REG_ECX -> frame[6]
		//   REG_EAX -> frame[7]
		//
		// This gives us a convenient trick. Changing one of these saved values
		// changes what POPAD restores later. So if the original instruction
		// wanted g_giveAll in EAX, GiveAll_Hk() changes frame[REG_EAX]. POPAD
		// then puts our selected pointer into EAX and the engine carries on as
		// if the original load had happened there.
		//
		// The same PUSHAD frame gives us the source register when the player is
		// already available in one.
		//
		// GiveAllSource::PlayerState means `location` is a REG_* value and the
		// corresponding register contains playerState_s*. For example:
		//
		//   { 0x5D8D22, 5, GiveAllSource::PlayerState, REG_EBX, REG_EAX }
		//
		// means EBX contains playerState_s* at 0x5D8D22 and the original
		// g_giveAll load writes to EAX.
		//
		// GiveAllSource::Entity is the same idea except that the source register
		// contains gentity_s*. For example:
		//
		//   { 0x5D8EA0, 5, GiveAllSource::Entity, REG_EDI, REG_EAX }
		//
		// means EDI contains gentity_s* at 0x5D8EA0. GiveAll_Hk() follows
		// ent->client to obtain the corresponding player state and then puts the
		// selected dvar pointer into the saved EAX slot.
		//
		// Note that the register name by itself tells us nothing about the type
		// of pointer stored there. EDI contains gentity_s* at some of these
		// sites and playerState_s* at others. This is simply what the surrounding
		// machine code tells us at each address. So don't infer the source type
		// from another entry that happens to use the same register.
		//
		// Then we have GiveAllSource::StackArgument. For example:
		//
		//   { 0x43AC60, 5, GiveAllSource::StackArgument, 4, REG_EAX }
		//
		// Here the playerState_s* is at [ESP + 4] as seen at 0x43AC60.
		//
		// The `4` deserves a bit of explanation since the StackArgument name can
		// make this look more abstract than it really is. What GiveAll_Hk()
		// actually has here is a byte offset from ESP at this exact instruction.
		//
		// The function may have adjusted ESP in its prologue, reserved stack
		// space, pushed some temporary value, and so on before reaching this
		// point. So the useful fact for us is simply that, at 0x43AC60, the
		// pointer is at [ESP + 4].
		//
		// GiveAll_Hk() gets back to that stack using frame + 9. Recall that
		// frame[8] is our CALL return address. Thus frame[9] corresponds to the
		// original [ESP + 0], and an offset of 4 refers to the DWORD at the
		// original [ESP + 4].
		//
		// In other words, the StackArgument case does approximately:
		//
		//   player = *reinterpret_cast<playerState_s**>(
		//       original_esp + site.location);
		//
		// And so:
		//
		//   { 0x496B0D, 5, GiveAllSource::StackArgument, 8, REG_EAX }
		//
		// means that the playerState_s* is at [ESP + 8] at 0x496B0D.
		//
		// Note that these offsets belong to the individual instruction sites.
		// A `4` here should not be read as some universal first-argument marker.
		// If the surrounding machine code changes, then the stack offset may
		// change with it.
		//
		// GiveAllSource::PredictedPlayerState is the easy case:
		//
		//   { 0x59EF21, 5, GiveAllSource::PredictedPlayerState, 0, REG_EAX }
		//
		// This code is operating on the local predicted player, so we just use:
		//
		//   &Game::cgArray->predictedPlayerState
		//
		// directly.
		//
		// The `0` is unused for this source. It is present since GiveAllSite has
		// one `location` field shared by all of the source kinds. REG_EAX still
		// matters since the original instruction expects the dvar pointer there.
		//
		// Now for the instruction size.
		//
		// Most entries have size 5, which means our five-byte CALL replaces the
		// complete instruction exactly. A few entries have size 6:
		//
		//   { 0x59ECD0, 6, GiveAllSource::PredictedPlayerState, 0, REG_ECX }
		//
		// Here the original instruction occupied six bytes. The CALL is still
		// five bytes, so PatchGiveAll() replaces the byte left over with a NOP.
		// After the stub returns we execute that NOP and then continue after the
		// original six-byte instruction.
		//
		// Note that we really do need to replace the complete instruction. The
		// sixth byte is part of the old instruction and has no useful meaning
		// once its first five bytes have become a CALL.
		//
		// There is a related little detail in GiveAll_Hk():
		//
		//   frame[8] - 5
		//
		// is how we recover the address of the current GiveAllSite.
		//
		// The `5` here is the size of our CALL, not `site.size`. frame[8] is the
		// return address pushed by that CALL, so subtracting five gets us back
		// to its first byte.
		//
		// For example, with the six-byte site at 0x59ECD0:
		//
		//   CALL occupies 0x59ECD0..0x59ECD4
		//   return address is 0x59ECD5
		//   NOP occupies 0x59ECD5
		//   engine continues at 0x59ECD6
		//
		// And so:
		//
		//   0x59ECD5 - 5 == 0x59ECD0
		//
		// This is why `site.size` is used when installing the patch but not when
		// looking up the site in GiveAll_Hk().
		//
		// There is another subtle point here. GiveAll_Hk() is changing the saved
		// register frame. The compiler can use the registers as it
		// sees fit inside the C++ function. POPAD is what copies our modified
		// saved state back into the registers visible to the game.
		//
		// So take this entry:
		//
		//   { 0x5DC4F7, 6, GiveAllSource::Entity, REG_EDI, REG_EBX }
		//
		// It says:
		//
		//   * At 0x5DC4F7, EDI contains gentity_s*.
		//
		//   * The instruction being replaced is six bytes long.
		//
		//   * The original instruction produces the g_giveAll pointer in EBX.
		//
		// GiveAll_Hk() gets the entity from frame[REG_EDI], finds the
		// corresponding player state, decides which dvar applies, and replaces
		// frame[REG_EBX] with that pointer. POPAD restores it into EBX. RET then
		// returns to the NOP at 0x5DC4FC, after which execution continues at
		// 0x5DC4FD.
		//
		//
		// Finally, the useful invariant to keep in mind is this: from the game's point
		// of view, the hook should look like the original g_giveAll load. All
		// saved registers we do not touch come back as they were. The register
		// that originally received g_giveAll still receives a dvar pointer. For
		// a client with give-all enabled that pointer happens to be GiveAllDvar.
		//
		// With that in mind the entries below can be read fairly mechanically:
		//
		//   { ..., GiveAllSource::PlayerState, REG_ESI, REG_EAX }
		//
		//     The player state is in ESI and the selected dvar goes in EAX.
		//
		//   { ..., GiveAllSource::Entity, REG_EDI, REG_EBX }
		//
		//     The entity is in EDI and the selected dvar goes in EBX.
		//
		//   { ..., GiveAllSource::StackArgument, 4, REG_EAX }
		//
		//     The player state is at [ESP + 4] and the selected dvar goes in
		//     EAX.
		//
		//   { ..., GiveAllSource::PredictedPlayerState, 0, REG_ECX }
		//
		//     Use the local predicted player state and put the selected dvar in
		//     ECX.
		//
		const GiveAllSite sites[] =
		{
			{ 0x43AC60, 5, GiveAllSource::StackArgument, 				4, 			 REG_EAX },
			{ 0x43ACD4, 5, GiveAllSource::PlayerState, 				  REG_ESI, REG_EAX },
			{ 0x48BB80, 5, GiveAllSource::PredictedPlayerState, 0, 			 REG_EAX },
			{ 0x496B0D, 5, GiveAllSource::StackArgument, 				8, 			 REG_EAX },
			{ 0x4AB530, 5, GiveAllSource::StackArgument, 				4, 			 REG_EAX },
			{ 0x4B38B0, 5, GiveAllSource::StackArgument, 				4, 			 REG_EAX },
			{ 0x4BB33A, 5, GiveAllSource::StackArgument, 				4, 			 REG_EAX },
			{ 0x4D8BA0, 5, GiveAllSource::StackArgument, 				4, 			 REG_EAX },
			{ 0x4E1493, 5, GiveAllSource::PlayerState, 					REG_EDI, REG_EAX },
			{ 0x4E79E0, 5, GiveAllSource::StackArgument, 				4,			 REG_EAX },
			{ 0x4FD1D7, 5, GiveAllSource::PlayerState, 					REG_EDI, REG_EAX },
			{ 0x5763C0, 5, GiveAllSource::PlayerState, 					REG_EDI, REG_EAX },
			{ 0x576406, 5, GiveAllSource::PlayerState, 					REG_EDI, REG_EAX },
			{ 0x59ECD0, 6, GiveAllSource::PredictedPlayerState, 0, 			 REG_ECX },
			{ 0x59EF21, 5, GiveAllSource::PredictedPlayerState, 0, 			 REG_EAX },
			{ 0x5A18D1, 5, GiveAllSource::StackArgument, 				8, 			 REG_EAX },
			{ 0x5D8D22, 5, GiveAllSource::PlayerState, 					REG_EBX, REG_EAX },
			{ 0x5D8EA0, 5, GiveAllSource::Entity, 							REG_EDI, REG_EAX },
			{ 0x5D8F7F, 5, GiveAllSource::PlayerState, 					REG_EBX, REG_EAX },
			{ 0x5D97CA, 5, GiveAllSource::Entity, 							REG_EBX, REG_EAX },
			{ 0x5D991E, 5, GiveAllSource::Entity, 							REG_ESI, REG_EAX },
			{ 0x5D99E5, 5, GiveAllSource::Entity, 							REG_EDI, REG_EAX },
			{ 0x5D9AAC, 5, GiveAllSource::PlayerState, 					REG_EBX, REG_EAX },
			{ 0x5D9B8D, 5, GiveAllSource::Entity, 							REG_ESI, REG_EAX },
			{ 0x5D9CA8, 5, GiveAllSource::Entity, 							REG_EDI, REG_EAX },
			{ 0x5DC4F7, 6, GiveAllSource::Entity, 							REG_EDI, REG_EBX },
			{ 0x5DC567, 6, GiveAllSource::Entity, 							REG_EDI, REG_EBX },
			{ 0x5DC5E7, 6, GiveAllSource::Entity, 							REG_EBX, REG_EDI },
			{ 0x5DC669, 6, GiveAllSource::Entity, 							REG_EBX, REG_EDI },
			{ 0x5DC7A4, 6, GiveAllSource::Entity, 							REG_EBP, REG_EDI },
			{ 0x5DC832, 6, GiveAllSource::Entity, 							REG_EBP, REG_EDI },
			{ 0x5E26E0, 5, GiveAllSource::Entity, 							REG_EBP, REG_EAX },
			{ 0x5E2C9F, 5, GiveAllSource::Entity, 							REG_EBX, REG_EAX },
			{ 0x5FED1A, 5, GiveAllSource::PlayerState, 					REG_EDI, REG_EAX },
			{ 0x5FEE80, 5, GiveAllSource::PlayerState, 					REG_ESI, REG_EAX },
		};

		for (const auto& site : sites)
		{
			GiveAllSites[site.address] = site;
			Utils::Hook(site.address, GiveAll_Stub, HOOK_CALL).install()->quick();

			if (site.size > 5)
			{
				Utils::Hook::Nop(site.address + 5, site.size - 5);
			}
		}

		for (const auto address : { 0x43574B, 0x4E14FE, 0x4F76BF, 0x5D9AE2, 0x5DB41E, 0x5E2C13 })
		{
			Utils::Hook(address, BG_GetAmmoPlayerMax_Hk, HOOK_CALL).install()->quick();
		}

		ServerCommands::OnCommand(23, [](const Command::Params* params)
		{
			const auto clientNum = std::strtoul(params->get(1), nullptr, 10);
			if (clientNum < GiveAllClients.size())
				GiveAllClients[clientNum] = std::strtol(params->get(2), nullptr, 10) != 0;

			return true;
		});

		Events::OnClientDisconnect([](const int clientNum)
		{
			if (static_cast<std::size_t>(clientNum) < GiveAllClients.size())
				GiveAllClients[clientNum] = false;
		});

		Events::OnVMShutdown([]
		{
			GiveAllClients.fill(false);
		});

		Events::OnCLDisconnected([]([[maybe_unused]] bool wasConnected)
		{
			GiveAllClients.fill(false);
		});

		Scheduler::Loop([]
		{
			if (!Dedicated::IsRunning())
				return;

			for (std::size_t i = 0; i < GiveAllClients.size(); ++i)
			{
				auto* ent = &Game::g_entities[i];
				if (ent->client && ent->client->sess.connected != Game::CON_DISCONNECTED && ent->health > 0 && HasGiveAll(&ent->client->ps))
					GiveAllAmmo(ent, false);
			}
		}, Scheduler::Pipeline::SERVER);
	}

	int ClientCommand::BG_GetAmmoPlayerMax_Hk(Game::playerState_s* ps, const unsigned int weaponIndex, const unsigned int weaponIndexToSkip)
	{
		const auto result = Utils::Hook::Call<int(Game::playerState_s*, unsigned int, unsigned int)>(0x4A5560)(ps, weaponIndex, weaponIndexToSkip);

		if (result || !HasGiveAll(ps) || weaponIndex == weaponIndexToSkip)
			return result;

		return Game::BG_GetWeaponDef(weaponIndex)->iMaxAmmo;
	}

	void ClientCommand::GiveAllAmmo(Game::gentity_s* ent, const bool refill)
	{
		auto& common = ent->client->ps.weapCommon;
		const auto weaponCount = Game::BG_GetNumWeapons();

		std::vector<unsigned int> weapons;
		const auto addWeapon = [&](const unsigned int weapon)
		{
			if (weapon && weapon < weaponCount && std::ranges::find(weapons, weapon) == weapons.end())
				weapons.push_back(weapon);
		};

		addWeapon(common.weapon);
		addWeapon(common.primaryWeaponForAltMode);
		addWeapon(static_cast<unsigned int>(common.offHandIndex));

		if (common.weapon && common.weapon < weaponCount)
		{
			addWeapon(Game::BG_GetWeaponCompleteDef(common.weapon)->altWeaponIndex);
		}

		const auto isAmmoInUse = [&](const int ammoType)
		{
			return std::ranges::any_of(weapons, [&](const unsigned int weapon)
			{
				return Game::BG_GetWeaponDef(weapon)->iAmmoIndex == ammoType;
			});
		};

		const auto isClipInUse = [&](const int clipIndex)
		{
			return std::ranges::any_of(weapons, [&](const unsigned int weapon)
			{
				return Game::BG_GetWeaponDef(weapon)->iClipIndex == clipIndex;
			});
		};

		for (const auto weapon : weapons)
		{
			const auto* weaponDef = Game::BG_GetWeaponDef(weapon);
			auto allocated = false;

			if (std::ranges::find(common.ammoNotInClip, weaponDef->iAmmoIndex, &Game::GlobalAmmo::ammoType) == std::end(common.ammoNotInClip))
			{
				auto* ammo = std::ranges::find(common.ammoNotInClip, 0, &Game::GlobalAmmo::ammoType);
				if (ammo == std::end(common.ammoNotInClip))
				{
					ammo = std::ranges::find_if(common.ammoNotInClip, [&](const Game::GlobalAmmo& entry)
					{
						return !isAmmoInUse(entry.ammoType);
					});
				}

				if (ammo != std::end(common.ammoNotInClip))
				{
					ammo->ammoType = weaponDef->iAmmoIndex;
					ammo->ammoCount = 0;
					allocated = true;
				}
			}

			if (std::ranges::find(common.ammoInClip, weaponDef->iClipIndex, &Game::ClipAmmo::clipIndex) == std::end(common.ammoInClip))
			{
				auto* clip = std::ranges::find(common.ammoInClip, 0, &Game::ClipAmmo::clipIndex);
				if (clip == std::end(common.ammoInClip))
				{
					clip = std::ranges::find_if(common.ammoInClip, [&](const Game::ClipAmmo& entry)
					{
						return !isClipInUse(entry.clipIndex);
					});
				}

				if (clip != std::end(common.ammoInClip))
				{
					clip->clipIndex = weaponDef->iClipIndex;
					clip->ammoCount[0] = 0;
					clip->ammoCount[1] = 0;
					allocated = true;
				}
			}

			if (allocated || refill)
				Game::Add_Ammo(ent, weapon, 0, 998, 1);
		}
	}

	ClientCommand::ClientCommand()
	{
		AssertOffset(Game::playerState_s, stats, 0x150);

		// Hook call to ClientCommand in SV_ExecuteClientCommand so we may add custom commands
		Utils::Hook(0x6259FA, ClientCommandStub, HOOK_CALL).install()->quick();

		CheatsEnabled = false;

		PatchGiveAll();
		AddCheatCommands();

		if (Dedicated::IsEnabled())
		{
			Events::OnSVInit(AddServerCommands);
		}

		AddScriptFunctions();
		AddScriptMethods();
#ifdef _DEBUG
		AddDevelopmentCommands();
#endif
	}
}
