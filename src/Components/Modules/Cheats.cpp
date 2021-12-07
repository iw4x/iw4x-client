#include "STDInclude.hpp"

namespace Components
{
	void Cheats::AddCheatCommands()
	{
		static int toastDurationShort = 1000;
		static int toastDurationMedium = 2500;
		static int toastDurationLong = 5000;

		Command::Add("noclip", [](Command::Params*)
		{
			int clientNum = Game::CG_GetClientNum();
			if (!Game::CL_IsCgameInitialized() || clientNum >= 18 || clientNum < 0 || !Game::g_entities[clientNum].client)
			{
				Logger::Print("You are not hosting a match!\n");
				Toast::Show("cardicon_stop", "Error", "You are not hosting a match!", toastDurationMedium);
				return;
			}

			if (!Dvar::Var("sv_cheats").get<bool>())
			{
				Logger::Print("Cheats disabled!\n");
				Toast::Show("cardicon_stop", "Error", "Cheats disabled!", toastDurationMedium);
				return;
			}

			Game::g_entities[clientNum].client->flags ^= Game::PLAYER_FLAG_NOCLIP;

			Logger::Print("Noclip toggled\n");
			Toast::Show("cardicon_abduction", "Success", "Noclip toggled", toastDurationShort);
		});

		Command::Add("ufo", [](Command::Params*)
		{
			int clientNum = Game::CG_GetClientNum();
			if (!Game::CL_IsCgameInitialized() || clientNum >= 18 || clientNum < 0 || !Game::g_entities[clientNum].client)
			{
				Logger::Print("You are not hosting a match!\n");
				Toast::Show("cardicon_stop", "Error", "You are not hosting a match!", toastDurationMedium);
				return;
			}

			if (!Dvar::Var("sv_cheats").get<bool>())
			{
				Logger::Print("Cheats disabled!\n");
				Toast::Show("cardicon_stop", "Error", "Cheats disabled!", toastDurationMedium);
				return;
			}

			Game::g_entities[clientNum].client->flags ^= Game::PLAYER_FLAG_UFO;

			Logger::Print("UFO toggled\n");
			Toast::Show("cardicon_abduction", "Success", "UFO toggled", toastDurationShort);
		});

		Command::Add("god", [](Command::Params*)
		{
			int clientNum = Game::CG_GetClientNum();
			if (!Game::CL_IsCgameInitialized() || clientNum >= 18 || clientNum < 0)
			{
				Logger::Print("You are not hosting a match!\n");
				Toast::Show("cardicon_stop", "Error", "You are not hosting a match!", toastDurationMedium);
				return;
			}

			if (!Dvar::Var("sv_cheats").get<bool>())
			{
				Logger::Print("Cheats disabled!\n");
				Toast::Show("cardicon_stop", "Error", "Cheats disabled!", toastDurationMedium);
				return;
			}

			Game::g_entities[clientNum].flags ^= Game::FL_GODMODE;

			Logger::Print("God toggled\n");
			Toast::Show("cardicon_abduction", "Success", "God toggled", toastDurationShort);
		});

		Command::Add("demigod", [](Command::Params*)
		{
			int clientNum = Game::CG_GetClientNum();
			if (!Game::CL_IsCgameInitialized() || clientNum >= 18 || clientNum < 0)
			{
				Logger::Print("You are not hosting a match!\n");
				Toast::Show("cardicon_stop", "Error", "You are not hosting a match!", toastDurationMedium);
				return;
			}

			if (!Dvar::Var("sv_cheats").get<bool>())
			{
				Logger::Print("Cheats disabled!\n");
				Toast::Show("cardicon_stop", "Error", "Cheats disabled!", toastDurationMedium);
				return;
			}

			Game::g_entities[clientNum].flags ^= Game::FL_DEMI_GODMODE;

			Logger::Print("Demigod toggled\n");
			Toast::Show("cardicon_abduction", "Success", "Demigod toggled", toastDurationShort);
		});

		Command::Add("notarget", [](Command::Params*)
		{
			int clientNum = Game::CG_GetClientNum();
			if (!Game::CL_IsCgameInitialized() || clientNum >= 18 || clientNum < 0)
			{
				Logger::Print("You are not hosting a match!\n");
				Toast::Show("cardicon_stop", "Error", "You are not hosting a match!", toastDurationMedium);
				return;
			}

			if (!Dvar::Var("sv_cheats").get<bool>())
			{
				Logger::Print("Cheats disabled!\n");
				Toast::Show("cardicon_stop", "Error", "Cheats disabled!", toastDurationMedium);
				return;
			}

			Game::g_entities[clientNum].flags ^= Game::FL_NOTARGET;

			Logger::Print("Notarget toggled\n");
			Toast::Show("cardicon_abduction", "Success", "Notarget toggled", toastDurationShort);
		});

		Command::Add("setviewpos", [](Command::Params* params)
		{
			int clientNum = Game::CG_GetClientNum();
			if (!Game::CL_IsCgameInitialized() || clientNum >= 18 || clientNum < 0 || !Game::g_entities[clientNum].client)
			{
				Logger::Print("You are not hosting a match!\n");
				Toast::Show("cardicon_stop", "Error", "You are not hosting a match!", toastDurationMedium);
				return;
			}

			if (!Dvar::Var("sv_cheats").get<bool>())
			{
				Logger::Print("Cheats disabled!\n");
				Toast::Show("cardicon_stop", "Error", "Cheats disabled!", toastDurationMedium);
				return;
			}

			if (params->length() != 4 && params->length() != 6)
			{
				Logger::Print("Invalid coordinate specified!\n");
				Toast::Show("cardicon_stop", "Error", "Invalid coordinate specified!", toastDurationMedium);
				return;
			}

			float pos[3] = { 0.0f, 0.0f, 0.0f };
			float orientation[3] = { 0.0f, 0.0f, 0.0f };

			pos[0] = strtof(params->get(1), nullptr);
			pos[1] = strtof(params->get(2), nullptr);
			pos[2] = strtof(params->get(3), nullptr);

			if (params->length() == 6)
			{
				orientation[0] = strtof(params->get(4), nullptr);
				orientation[1] = strtof(params->get(5), nullptr);
			}

			Game::TeleportPlayer(&Game::g_entities[clientNum], pos, orientation);

			// Logging will spam the console and screen if people use cinematics
		});
	}

	void Cheats::AddScriptFunctions()
	{
		Script::AddFunction("Noclip", [](Game::scr_entref_t entref) // gsc: Noclip(<optional int toggle>);
		{
			if (entref >= Game::MAX_GENTITIES || Game::g_entities[entref].client == nullptr)
			{
				Game::Scr_Error(Utils::String::VA("^1NoClip: entity %u is not a client\n", entref));
				return;
			}

			if (Game::Scr_GetNumParam() == 1u && Game::Scr_GetType(0) == Game::VAR_INTEGER)
			{
				if (Game::Scr_GetInt(0))
				{
					Game::g_entities[entref].client->flags |= Game::PLAYER_FLAG_NOCLIP;
				}
				else
				{
					Game::g_entities[entref].client->flags &= ~Game::PLAYER_FLAG_NOCLIP;
				}
			}
			else
			{
				Game::g_entities[entref].client->flags ^= Game::PLAYER_FLAG_NOCLIP;
			}
		});

		Script::AddFunction("Ufo", [](Game::scr_entref_t entref) // gsc: Ufo(<optional int toggle>);
		{
			if (entref >= Game::MAX_GENTITIES || Game::g_entities[entref].client == nullptr)
			{
				Game::Scr_Error(Utils::String::VA("^1Ufo: entity %u is not a client\n", entref));
				return;
			}

			if (Game::Scr_GetNumParam() == 1u && Game::Scr_GetType(0) == Game::VAR_INTEGER)
			{
				if (Game::Scr_GetInt(0))
				{
					Game::g_entities[entref].client->flags |= Game::PLAYER_FLAG_UFO;
				}
				else
				{
					Game::g_entities[entref].client->flags &= ~Game::PLAYER_FLAG_UFO;
				}
			}
			else
			{
				Game::g_entities[entref].client->flags ^= Game::PLAYER_FLAG_UFO;
			}
		});

		Script::AddFunction("God", [](Game::scr_entref_t entref) // gsc: God(<optional int toggle>);
		{
			if (entref >= Game::MAX_GENTITIES)
			{
				Game::Scr_Error(Utils::String::VA("^1God: entity %u is out of bounds\n", entref));
				return;
			}

			if (Game::Scr_GetNumParam() == 1u && Game::Scr_GetType(0) == Game::VAR_INTEGER)
			{
				if (Game::Scr_GetInt(0))
				{
					Game::g_entities[entref].flags |= Game::FL_GODMODE;
				}
				else
				{
					Game::g_entities[entref].flags &= ~Game::FL_GODMODE;
				}
			}
			else
			{
				Game::g_entities[entref].flags ^= Game::FL_GODMODE;
			}
		});

		Script::AddFunction("Demigod", [](Game::scr_entref_t entref) // gsc: Demigod(<optional int toggle>);
		{
			if (entref >= Game::MAX_GENTITIES)
			{
				Game::Scr_Error(Utils::String::VA("^1Demigod: entity %u is out of bounds\n", entref));
				return;
			}

			if (Game::Scr_GetNumParam() == 1u && Game::Scr_GetType(0) == Game::VAR_INTEGER)
			{
				if (Game::Scr_GetInt(0))
				{
					Game::g_entities[entref].flags |= Game::FL_DEMI_GODMODE;
				}
				else
				{
					Game::g_entities[entref].flags &= ~Game::FL_DEMI_GODMODE;
				}
			}
			else
			{
				Game::g_entities[entref].flags ^= Game::FL_DEMI_GODMODE;
			}
		});

		Script::AddFunction("Notarget", [](Game::scr_entref_t entref) // gsc: Demigod(<optional int toggle>);
		{
			if (entref >= Game::MAX_GENTITIES)
			{
				Game::Scr_Error(Utils::String::VA("^1Notarget: entity %u is out of bounds\n", entref));
				return;
			}

			if (Game::Scr_GetNumParam() == 1u && Game::Scr_GetType(0) == Game::VAR_INTEGER)
			{
				if (Game::Scr_GetInt(0))
				{
					Game::g_entities[entref].flags |= Game::FL_NOTARGET;
				}
				else
				{
					Game::g_entities[entref].flags &= ~Game::FL_NOTARGET;
				}
			}
			else
			{
				Game::g_entities[entref].flags ^= Game::FL_NOTARGET;
			}
		});
	}

	Cheats::Cheats()
	{
		// Disable native cheat commands
		Utils::Hook::Nop(0x474846, 5); // Cmd_Noclip_f
		Utils::Hook::Nop(0x474859, 5); // Cmd_UFO_f
		Utils::Hook::Nop(0x47480A, 5); // Cmd_God_f
		Utils::Hook::Nop(0x47481D, 5); // Cmd_DemiGod_f
		Utils::Hook::Nop(0x474833, 5); // Cmd_Notarget_f

		Cheats::AddCheatCommands();
		Cheats::AddScriptFunctions();
	}

	Cheats::~Cheats()
	{
	}
}
