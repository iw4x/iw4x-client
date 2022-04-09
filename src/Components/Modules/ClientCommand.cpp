#include <STDInclude.hpp>

namespace Components
{
	std::unordered_map<std::string, Utils::Slot<ClientCommand::Callback>> ClientCommand::FunctionMap;

	bool ClientCommand::CheatsOk(const Game::gentity_s* ent)
	{
		const auto entNum = ent->s.number;

		if (!Dvar::Var("sv_cheats").get<bool>())
		{
			Logger::Print("CheatsOk: cheats are disabled!\n");
			Game::SV_GameSendServerCommand(entNum, 0, Utils::String::VA("%c \"GAME_CHEATSNOTENABLED\"", 0x65));
			return false;
		}

		if (ent->health < 1)
		{
			Logger::Print("CheatsOk: entity %i must be alive to use this command!\n", entNum);
			Game::SV_GameSendServerCommand(entNum, 0, Utils::String::VA("%c \"GAME_MUSTBEALIVECOMMAND\"", 0x65));
			return false;
		}

		return true;
	}

	bool ClientCommand::CallbackHandler(Game::gentity_s* ent, const char* cmd)
	{
		const auto command = Utils::String::ToLower(cmd);
		const auto got = ClientCommand::FunctionMap.find(command);

		if (got != ClientCommand::FunctionMap.end())
		{
			got->second(ent);
			return true;
		}

		return false;
	}

	void ClientCommand::Add(const char* name, Utils::Slot<Callback> callback)
	{
		const auto command = Utils::String::ToLower(name);

		ClientCommand::FunctionMap[command] = std::move(callback);
	}

	void ClientCommand::ClientCommandStub(const int clientNum)
	{
		char cmd[1024]{};
		const auto entity = &Game::g_entities[clientNum];

		if (entity->client == nullptr)
		{
			Logger::Print("ClientCommand: client %d is not fully in game yet\n", clientNum);
			return;
		}

		Game::SV_Cmd_ArgvBuffer(0, cmd, sizeof(cmd));

		if (!ClientCommand::CallbackHandler(entity, cmd))
		{
			// If no callback was found call original game function
			Utils::Hook::Call<void(const int)>(0x416790)(clientNum);
		}
	}

	void ClientCommand::AddCheatCommands()
	{
		ClientCommand::Add("noclip", [](Game::gentity_s* ent)
		{
			if (!ClientCommand::CheatsOk(ent))
				return;

			ent->client->flags ^= Game::PLAYER_FLAG_NOCLIP;

			const auto entNum = ent->s.number;
			Logger::Print("Noclip toggled for entity %i\n", entNum);

			Game::SV_GameSendServerCommand(entNum, 0, Utils::String::VA("%c \"%s\"", 0x65,
				(ent->client->flags & Game::PLAYER_FLAG_NOCLIP) ? "GAME_NOCLIPON" : "GAME_NOCLIPOFF"));
		});

		ClientCommand::Add("ufo", [](Game::gentity_s* ent)
		{
			if (!ClientCommand::CheatsOk(ent))
				return;

			ent->client->flags ^= Game::PLAYER_FLAG_UFO;

			const auto entNum = ent->s.number;
			Logger::Print("UFO toggled for entity %i\n", entNum);

			Game::SV_GameSendServerCommand(entNum, 0, Utils::String::VA("%c \"%s\"", 0x65,
				(ent->client->flags & Game::PLAYER_FLAG_UFO) ? "GAME_UFOON" : "GAME_UFOOFF"));
		});

		ClientCommand::Add("god", [](Game::gentity_s* ent)
		{
			if (!ClientCommand::CheatsOk(ent))
				return;

			ent->flags ^= Game::FL_GODMODE;

			const auto entNum = ent->s.number;
			Logger::Print("God toggled for entity %i\n", entNum);

			Game::SV_GameSendServerCommand(entNum, 0, Utils::String::VA("%c \"%s\"", 0x65,
				(ent->flags & Game::FL_GODMODE) ? "GAME_GODMODE_ON" : "GAME_GODMODE_OFF"));
		});

		ClientCommand::Add("demigod", [](Game::gentity_s* ent)
		{
			if (!ClientCommand::CheatsOk(ent))
				return;

			ent->flags ^= Game::FL_DEMI_GODMODE;

			const auto entNum = ent->s.number;
			Logger::Print("Demigod toggled for entity %i\n", entNum);

			Game::SV_GameSendServerCommand(entNum, 0, Utils::String::VA("%c \"%s\"", 0x65,
				(ent->flags & Game::FL_DEMI_GODMODE) ? "GAME_DEMI_GODMODE_ON" : "GAME_DEMI_GODMODE_OFF"));
		});

		ClientCommand::Add("notarget", [](Game::gentity_s* ent)
		{
			if (!ClientCommand::CheatsOk(ent))
				return;

			ent->flags ^= Game::FL_NOTARGET;

			const auto entNum = ent->s.number;
			Logger::Print("Notarget toggled for entity %i\n", entNum);

			Game::SV_GameSendServerCommand(entNum, 0, Utils::String::VA("%c \"%s\"", 0x65,
				(ent->flags & Game::FL_NOTARGET) ? "GAME_NOTARGETON" : "GAME_NOTARGETOFF"));
		});

		ClientCommand::Add("setviewpos", [](Game::gentity_s* ent)
		{
			assert(ent != nullptr);

			if (!ClientCommand::CheatsOk(ent))
				return;

			Command::ServerParams params = {};
			Game::vec3_t origin, angles{0.f, 0.f, 0.f};

			if (params.size() < 4 || params.size() > 6)
			{
				Game::SV_GameSendServerCommand(ent->s.number, 0,
					Utils::String::VA("%c \"GAME_USAGE\x15: setviewpos x y z [yaw] [pitch]\n\"", 0x65));
				return;
			}

			for (auto i = 0; i < 3; i++)
			{
				origin[i] = std::strtof(params.get(i + 1), nullptr);
			}

			if (params.size() >= 5)
			{
				angles[1] = std::strtof(params.get(4), nullptr); // Yaw
			}

			if (params.size() == 6)
			{
				angles[0] = std::strtof(params.get(5), nullptr); // Pitch
			}

			Game::TeleportPlayer(ent, origin, angles);
		});
	}

	void ClientCommand::AddScriptFunctions()
	{
		Script::AddFunction("Noclip", [](Game::scr_entref_t entref) // gsc: Noclip(<optional int toggle>);
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

		Script::AddFunction("Ufo", [](Game::scr_entref_t entref) // gsc: Ufo(<optional int toggle>);
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

		Script::AddFunction("God", [](Game::scr_entref_t entref) // gsc: God(<optional int toggle>);
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

		Script::AddFunction("Demigod", [](Game::scr_entref_t entref) // gsc: Demigod(<optional int toggle>);
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

		Script::AddFunction("Notarget", [](Game::scr_entref_t entref) // gsc: Notarget(<optional int toggle>);
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
	}

	ClientCommand::ClientCommand()
	{
		// Hook call to ClientCommand in SV_ExecuteClientCommand so we may add custom commands
		Utils::Hook(0x6259FA, ClientCommand::ClientCommandStub, HOOK_CALL).install()->quick();

		ClientCommand::AddCheatCommands();
		ClientCommand::AddScriptFunctions();
	}
}
