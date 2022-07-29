#include <STDInclude.hpp>
#include "GSC/Script.hpp"

namespace Components
{
	std::unordered_map<std::string, std::function<void(Game::gentity_s*, Command::ServerParams*)>> ClientCommand::HandlersSV;

	bool ClientCommand::CheatsOk(const Game::gentity_s* ent)
	{
		const auto entNum = ent->s.number;

		if (!Dvar::Var("sv_cheats").get<bool>())
		{
			Logger::Debug("Cheats are disabled!");
			Game::SV_GameSendServerCommand(entNum, Game::SV_CMD_CAN_IGNORE, Utils::String::VA("%c \"GAME_CHEATSNOTENABLED\"", 0x65));
			return false;
		}

		if (ent->health < 1)
		{
			Logger::Debug("Entity {} must be alive to use this command!", entNum);
			Game::SV_GameSendServerCommand(entNum, Game::SV_CMD_CAN_IGNORE, Utils::String::VA("%c \"GAME_MUSTBEALIVECOMMAND\"", 0x65));
			return false;
		}

		return true;
	}

	void ClientCommand::Add(const char* name, const std::function<void(Game::gentity_s*, Command::ServerParams*)>& callback)
	{
		const auto command = Utils::String::ToLower(name);

		ClientCommand::HandlersSV[command] = callback;
	}

	void ClientCommand::ClientCommandStub(const int clientNum)
	{
		const auto ent = &Game::g_entities[clientNum];

		if (ent->client == nullptr)
		{
			Logger::Debug("ClientCommand: client {} is not fully in game yet", clientNum);
			return;
		}

		Command::ServerParams params;
		const auto command = Utils::String::ToLower(params.get(0));

		if (const auto got = HandlersSV.find(command); got != HandlersSV.end())
		{
			got->second(ent, &params);
			return;
		}

		Utils::Hook::Call<void(int)>(0x416790)(clientNum);
	}

	void ClientCommand::AddCheatCommands()
	{
		ClientCommand::Add("noclip", [](Game::gentity_s* ent, [[maybe_unused]] Command::ServerParams* params)
		{
			if (!ClientCommand::CheatsOk(ent))
				return;

			ent->client->flags ^= Game::PLAYER_FLAG_NOCLIP;

			const auto entNum = ent->s.number;
			Logger::Debug("Noclip toggled for entity {}", entNum);

			Game::SV_GameSendServerCommand(entNum, Game::SV_CMD_CAN_IGNORE, Utils::String::VA("%c \"%s\"", 0x65,
				(ent->client->flags & Game::PLAYER_FLAG_NOCLIP) ? "GAME_NOCLIPON" : "GAME_NOCLIPOFF"));
		});

		ClientCommand::Add("ufo", [](Game::gentity_s* ent, [[maybe_unused]] Command::ServerParams* params)
		{
			if (!ClientCommand::CheatsOk(ent))
				return;

			ent->client->flags ^= Game::PLAYER_FLAG_UFO;

			const auto entNum = ent->s.number;
			Logger::Debug("UFO toggled for entity {}", entNum);

			Game::SV_GameSendServerCommand(entNum, Game::SV_CMD_CAN_IGNORE, Utils::String::VA("%c \"%s\"", 0x65,
				(ent->client->flags & Game::PLAYER_FLAG_UFO) ? "GAME_UFOON" : "GAME_UFOOFF"));
		});

		ClientCommand::Add("god", [](Game::gentity_s* ent, [[maybe_unused]] Command::ServerParams* params)
		{
			if (!ClientCommand::CheatsOk(ent))
				return;

			ent->flags ^= Game::FL_GODMODE;

			const auto entNum = ent->s.number;
			Logger::Debug("God toggled for entity {}", entNum);

			Game::SV_GameSendServerCommand(entNum, Game::SV_CMD_CAN_IGNORE, Utils::String::VA("%c \"%s\"", 0x65,
				(ent->flags & Game::FL_GODMODE) ? "GAME_GODMODE_ON" : "GAME_GODMODE_OFF"));
		});

		ClientCommand::Add("demigod", [](Game::gentity_s* ent, [[maybe_unused]] Command::ServerParams* params)
		{
			if (!ClientCommand::CheatsOk(ent))
				return;

			ent->flags ^= Game::FL_DEMI_GODMODE;

			const auto entNum = ent->s.number;
			Logger::Debug("Demigod toggled for entity {}", entNum);

			Game::SV_GameSendServerCommand(entNum, Game::SV_CMD_CAN_IGNORE, Utils::String::VA("%c \"%s\"", 0x65,
				(ent->flags & Game::FL_DEMI_GODMODE) ? "GAME_DEMI_GODMODE_ON" : "GAME_DEMI_GODMODE_OFF"));
		});

		ClientCommand::Add("notarget", [](Game::gentity_s* ent, [[maybe_unused]] Command::ServerParams* params)
		{
			if (!ClientCommand::CheatsOk(ent))
				return;

			ent->flags ^= Game::FL_NOTARGET;

			const auto entNum = ent->s.number;
			Logger::Debug("Notarget toggled for entity {}", entNum);

			Game::SV_GameSendServerCommand(entNum, Game::SV_CMD_CAN_IGNORE, Utils::String::VA("%c \"%s\"", 0x65,
				(ent->flags & Game::FL_NOTARGET) ? "GAME_NOTARGETON" : "GAME_NOTARGETOFF"));
		});

		ClientCommand::Add("setviewpos", [](Game::gentity_s* ent, [[maybe_unused]] Command::ServerParams* params)
		{
			assert(ent != nullptr);

			if (!ClientCommand::CheatsOk(ent))
				return;

			Game::vec3_t origin, angles{0.f, 0.f, 0.f};

			if (params->size() < 4 || params->size() > 6)
			{
				Game::SV_GameSendServerCommand(ent->s.number, Game::SV_CMD_CAN_IGNORE,
					Utils::String::VA("%c \"GAME_USAGE\x15: setviewpos x y z [yaw] [pitch]\n\"", 0x65));
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

		ClientCommand::Add("give", [](Game::gentity_s* ent, [[maybe_unused]] Command::ServerParams* params)
		{
			if (!ClientCommand::CheatsOk(ent))
				return;

			if (params->size() < 2)
			{
				Game::SV_GameSendServerCommand(ent->s.number, Game::SV_CMD_CAN_IGNORE,
					Utils::String::VA("%c \"GAME_USAGE\x15: give <weapon name>\"", 0x65));
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
				if (index != 0)
				{
					Game::Add_Ammo(ent, index, 0, 998, 1);
				}
			}
		});
	}

	void ClientCommand::AddDevelopmentCommands()
	{
		ClientCommand::Add("dropallbots", []([[maybe_unused]] Game::gentity_s* ent, [[maybe_unused]] Command::ServerParams* params)
		{
			Game::SV_DropAllBots();
		});

		ClientCommand::Add("entitylist", []([[maybe_unused]] Game::gentity_s* ent, [[maybe_unused]] Command::ServerParams* params)
		{
			Game::Svcmd_EntityList_f();
		});

		ClientCommand::Add("printentities", []([[maybe_unused]] Game::gentity_s* ent, [[maybe_unused]] Command::ServerParams* params)
		{
			Game::G_PrintEntities();
		});

		ClientCommand::Add("entitycount", []([[maybe_unused]] Game::gentity_s* ent, [[maybe_unused]] Command::ServerParams* params)
		{
			Logger::Print("Entity count = {}\n", Game::level->num_entities);
		});

		// Also known as: "vis"
		ClientCommand::Add("visionsetnaked", []([[maybe_unused]] Game::gentity_s* ent, [[maybe_unused]] Command::ServerParams* params)
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

			assert(ent->client != nullptr);

			constexpr auto visMode = Game::visionSetMode_t::VISIONSET_NORMAL;
			const auto* name = params->get(1);

			ent->client->visionDuration[visMode] = duration;
			strncpy_s(ent->client->visionName[visMode],
				sizeof(Game::gclient_t::visionName[0]) / sizeof(char), name, _TRUNCATE);

			Game::SV_GameSendServerCommand(ent->s.number, Game::SV_CMD_RELIABLE,
				Utils::String::VA("%c \"%s\" %i", Game::MY_CMDS[visMode], name, duration));
		});

		ClientCommand::Add("visionsetnight", []([[maybe_unused]] Game::gentity_s* ent, [[maybe_unused]] Command::ServerParams* params)
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

			assert(ent->client != nullptr);

			constexpr auto visMode = Game::visionSetMode_t::VISIONSET_NIGHT;
			const auto* name = params->get(1);

			ent->client->visionDuration[visMode] = duration;
			strncpy_s(ent->client->visionName[visMode],
				sizeof(Game::gclient_t::visionName[0]) / sizeof(char), name, _TRUNCATE);

			Game::SV_GameSendServerCommand(ent->s.number, Game::SV_CMD_RELIABLE,
				Utils::String::VA("%c \"%s\" %i", Game::MY_CMDS[visMode], name, duration));
		});

		ClientCommand::Add("g_testCmd", []([[maybe_unused]] Game::gentity_s* ent, [[maybe_unused]] Command::ServerParams* params)
		{
			assert(ent != nullptr);

			ent->client->ps.stunTime = 1000 + Game::level->time; // 1000 is the default test stun time
			Logger::Debug("playerState_s.stunTime is {}", ent->client->ps.stunTime);
		});

		ClientCommand::Add("kill", []([[maybe_unused]] Game::gentity_s* ent, [[maybe_unused]] Command::ServerParams* params)
		{
			assert(ent->client != nullptr);
			assert(ent->client->sess.connected != Game::CON_DISCONNECTED);

			if (ent->client->sess.sessionState != Game::SESS_STATE_PLAYING || !ClientCommand::CheatsOk(ent))
				return;

			Scheduler::Once([ent]
			{
				ent->flags &= ~(Game::FL_GODMODE | Game::FL_DEMI_GODMODE);
				ent->health = 0;
				ent->client->ps.stats[0] = 0;
				Game::player_die(ent, ent, ent, 100000, 12, 0, nullptr, Game::HITLOC_NONE, 0);
			}, Scheduler::Pipeline::SERVER);
		});
	}

	void ClientCommand::AddScriptFunctions()
	{
		Script::AddFunction("DropAllBots", [] // gsc: DropAllBots();
		{
			Game::SV_DropAllBots();
		});
	}

	ClientCommand::ClientCommand()
	{
		// Hook call to ClientCommand in SV_ExecuteClientCommand so we may add custom commands
		Utils::Hook(0x6259FA, ClientCommand::ClientCommandStub, HOOK_CALL).install()->quick();

		ClientCommand::AddCheatCommands();
		ClientCommand::AddScriptFunctions();
#ifdef _DEBUG
		ClientCommand::AddDevelopmentCommands();
#endif
	}
}
