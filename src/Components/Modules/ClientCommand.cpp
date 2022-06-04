#include <STDInclude.hpp>

namespace Components
{
	std::unordered_map<std::string, std::function<void(Game::gentity_s*, Command::ServerParams*)>> ClientCommand::HandlersSV;

	bool ClientCommand::CheatsOk(const Game::gentity_s* ent)
	{
		const auto entNum = ent->s.number;

		if (!Dvar::Var("sv_cheats").get<bool>())
		{
			Logger::Print("CheatsOk: cheats are disabled!\n");
			Game::SV_GameSendServerCommand(entNum, Game::SV_CMD_CAN_IGNORE, Utils::String::VA("%c \"GAME_CHEATSNOTENABLED\"", 0x65));
			return false;
		}

		if (ent->health < 1)
		{
			Logger::Print("CheatsOk: entity %i must be alive to use this command!\n", entNum);
			Game::SV_GameSendServerCommand(entNum, Game::SV_CMD_CAN_IGNORE, Utils::String::VA("%c \"GAME_MUSTBEALIVECOMMAND\"", 0x65));
			return false;
		}

		return true;
	}

	void ClientCommand::Add(const char* name, std::function<void(Game::gentity_s*, Command::ServerParams*)> callback)
	{
		const auto command = Utils::String::ToLower(name);

		ClientCommand::HandlersSV[command] = std::move(callback);
	}

	void ClientCommand::ClientCommandStub(const int clientNum)
	{
		const auto ent = &Game::g_entities[clientNum];

		if (ent->client == nullptr)
		{
			Logger::Print("ClientCommand: client %d is not fully in game yet\n", clientNum);
			return;
		}

		Command::ServerParams params;
		const auto command = Utils::String::ToLower(params.get(0));

		if (const auto got = HandlersSV.find(command); got != HandlersSV.end())
		{
			got->second(ent, &params);
			return;
		}

		Utils::Hook::Call<void(const int)>(0x416790)(clientNum);
	}

	void ClientCommand::AddCheatCommands()
	{
		ClientCommand::Add("noclip", [](Game::gentity_s* ent, [[maybe_unused]] Command::ServerParams* params)
		{
			if (!ClientCommand::CheatsOk(ent))
				return;

			ent->client->flags ^= Game::PLAYER_FLAG_NOCLIP;

			const auto entNum = ent->s.number;
			Logger::Print("Noclip toggled for entity %i\n", entNum);

			Game::SV_GameSendServerCommand(entNum, Game::SV_CMD_CAN_IGNORE, Utils::String::VA("%c \"%s\"", 0x65,
				(ent->client->flags & Game::PLAYER_FLAG_NOCLIP) ? "GAME_NOCLIPON" : "GAME_NOCLIPOFF"));
		});

		ClientCommand::Add("ufo", [](Game::gentity_s* ent, [[maybe_unused]] Command::ServerParams* params)
		{
			if (!ClientCommand::CheatsOk(ent))
				return;

			ent->client->flags ^= Game::PLAYER_FLAG_UFO;

			const auto entNum = ent->s.number;
			Logger::Print("UFO toggled for entity %i\n", entNum);

			Game::SV_GameSendServerCommand(entNum, Game::SV_CMD_CAN_IGNORE, Utils::String::VA("%c \"%s\"", 0x65,
				(ent->client->flags & Game::PLAYER_FLAG_UFO) ? "GAME_UFOON" : "GAME_UFOOFF"));
		});

		ClientCommand::Add("god", [](Game::gentity_s* ent, [[maybe_unused]] Command::ServerParams* params)
		{
			if (!ClientCommand::CheatsOk(ent))
				return;

			ent->flags ^= Game::FL_GODMODE;

			const auto entNum = ent->s.number;
			Logger::Print("God toggled for entity %i\n", entNum);

			Game::SV_GameSendServerCommand(entNum, Game::SV_CMD_CAN_IGNORE, Utils::String::VA("%c \"%s\"", 0x65,
				(ent->flags & Game::FL_GODMODE) ? "GAME_GODMODE_ON" : "GAME_GODMODE_OFF"));
		});

		ClientCommand::Add("demigod", [](Game::gentity_s* ent, [[maybe_unused]] Command::ServerParams* params)
		{
			if (!ClientCommand::CheatsOk(ent))
				return;

			ent->flags ^= Game::FL_DEMI_GODMODE;

			const auto entNum = ent->s.number;
			Logger::Print("Demigod toggled for entity %i\n", entNum);

			Game::SV_GameSendServerCommand(entNum, Game::SV_CMD_CAN_IGNORE, Utils::String::VA("%c \"%s\"", 0x65,
				(ent->flags & Game::FL_DEMI_GODMODE) ? "GAME_DEMI_GODMODE_ON" : "GAME_DEMI_GODMODE_OFF"));
		});

		ClientCommand::Add("notarget", [](Game::gentity_s* ent, [[maybe_unused]] Command::ServerParams* params)
		{
			if (!ClientCommand::CheatsOk(ent))
				return;

			ent->flags ^= Game::FL_NOTARGET;

			const auto entNum = ent->s.number;
			Logger::Print("Notarget toggled for entity %i\n", entNum);

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

			Game::TeleportPlayer(ent, origin, angles);
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
			Logger::Print("Entity count = %i\n", Game::level->num_entities);
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
		});
	}

	void ClientCommand::AddScriptFunctions()
	{
		Script::AddMethod("Noclip", [](Game::scr_entref_t entref) // gsc: Noclip(<optional int toggle>);
		{
			const auto* ent = Game::GetPlayerEntity(entref);

			if (Game::Scr_GetNumParam() >= 1u)
			{
				if (Game::Scr_GetInt(0))
				{
					ent->client->flags |= Game::PLAYER_FLAG_NOCLIP;
				}
				else
				{
					ent->client->flags &= ~Game::PLAYER_FLAG_NOCLIP;
				}
			}
			else
			{
				ent->client->flags ^= Game::PLAYER_FLAG_NOCLIP;
			}
		});

		Script::AddMethod("Ufo", [](Game::scr_entref_t entref) // gsc: Ufo(<optional int toggle>);
		{
			const auto* ent = Game::GetPlayerEntity(entref);

			if (Game::Scr_GetNumParam() >= 1u)
			{
				if (Game::Scr_GetInt(0))
				{
					ent->client->flags |= Game::PLAYER_FLAG_UFO;
				}
				else
				{
					ent->client->flags &= ~Game::PLAYER_FLAG_UFO;
				}
			}
			else
			{
				ent->client->flags ^= Game::PLAYER_FLAG_UFO;
			}
		});

		Script::AddMethod("God", [](Game::scr_entref_t entref) // gsc: God(<optional int toggle>);
		{
			auto* ent = Game::GetEntity(entref);

			if (Game::Scr_GetNumParam() >= 1u)
			{
				if (Game::Scr_GetInt(0))
				{
					ent->flags |= Game::FL_GODMODE;
				}
				else
				{
					ent->flags &= ~Game::FL_GODMODE;
				}
			}
			else
			{
				ent->flags ^= Game::FL_GODMODE;
			}
		});

		Script::AddMethod("Demigod", [](Game::scr_entref_t entref) // gsc: Demigod(<optional int toggle>);
		{
			auto* ent = Game::GetEntity(entref);

			if (Game::Scr_GetNumParam() >= 1u)
			{
				if (Game::Scr_GetInt(0))
				{
					ent->flags |= Game::FL_DEMI_GODMODE;
				}
				else
				{
					ent->flags &= ~Game::FL_DEMI_GODMODE;
				}
			}
			else
			{
				ent->flags ^= Game::FL_DEMI_GODMODE;
			}
		});

		Script::AddMethod("Notarget", [](Game::scr_entref_t entref) // gsc: Notarget(<optional int toggle>);
		{
			auto* ent = Game::GetEntity(entref);

			if (Game::Scr_GetNumParam() >= 1u)
			{
				if (Game::Scr_GetInt(0))
				{
					ent->flags |= Game::FL_NOTARGET;
				}
				else
				{
					ent->flags &= ~Game::FL_NOTARGET;
				}
			}
			else
			{
				ent->flags ^= Game::FL_NOTARGET;
			}
		});

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
