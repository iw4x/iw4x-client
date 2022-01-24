#include "STDInclude.hpp"

namespace Components
{
	std::vector<std::string> Bots::BotNames;
	Game::dvar_t* Bots::SVBotWarfare;

	struct BotMovementInfo
	{
		int buttons; // Actions
		int8_t forward;
		int8_t right;
		uint16_t weapon;
	};

	static BotMovementInfo g_botai[18];

	struct BotAction
	{
		const char* action;
		int key;
	};

	static const BotAction BotActions[] =
	{
		{ "gostand", Bots::JUMP },
		{ "gocrouch", Bots::CROUCH },
		{ "goprone", Bots::PRONE },
		{ "fire", Bots::FIRE },
		{ "melee", Bots::FIRE},
		{ "frag", Bots::THROW_EQUIPMENT },
		{ "smoke", Bots::THROW_SPECIAL },
		{ "reload", Bots::RELOAD },
		{ "sprint", Bots::SPRINT },
		{ "leanleft", Bots::LEAN_LEFT},
		{ "leanright", Bots::LEAN_RIGHT },
		{ "ads", Bots::ADS_MODE },
		{ "holdbreath", Bots::HOLD_BREATH },
		{ "use", Bots::USE },
		{ "0", Bots::NUM_0 },
		{ "1", Bots::NUM_1 },
		{ "2", Bots::NUM_2 },
		{ "3", Bots::NUM_3 },
		{ "4", Bots::NUM_4},
		{ "5", Bots::NUM_5 },
		{ "6", Bots::NUM_6 },
		{ "7", Bots::NUM_7 },
		{ "8", Bots::NUM_8 },
		{ "9", Bots::NUM_9 },
	};

	void Bots::BuildConnectString(char* buffer, const char* connectString, int num, int, int protocol, int checksum, int statVer, int statStuff, int port)
	{
		static int botId = 0;
		const char* botName;

		if (Bots::BotNames.empty())
		{
			FileSystem::File bots("bots.txt");

			if (bots.exists())
			{
				auto names = Utils::String::Explode(bots.getBuffer(), '\n');

				for (auto& name : names)
				{
					Utils::String::Replace(name, "\r", "");
					name = Utils::String::Trim(name);

					if (!name.empty())
					{
						Bots::BotNames.push_back(name);
					}
				}
			}
		}

		if (!Bots::BotNames.empty())
		{
			botId %= Bots::BotNames.size();
			botName = Bots::BotNames[botId++].data();
		}
		else
		{
			botName = Utils::String::VA("bot%d", ++botId);
		}

		_snprintf_s(buffer, 0x400, _TRUNCATE, connectString, num, botName, protocol, checksum, statVer, statStuff, port);
	}

	void Bots::Spawn(unsigned int count)
	{
		for (auto i = 0u; i < count; ++i)
		{
			Scheduler::OnDelay([]()
			{
				auto* ent = Game::SV_AddTestClient();
				if (ent == nullptr)
					return;

				Scheduler::OnDelay([ent]()
				{
					Game::Scr_AddString("autoassign");
					Game::Scr_AddString("team_marinesopfor");
					Game::Scr_Notify(ent, Game::SL_GetString("menuresponse", 0), 2);

					Scheduler::OnDelay([ent]()
					{
						Game::Scr_AddString(Utils::String::VA("class%u", Utils::Cryptography::Rand::GenerateInt() % 5u));
						Game::Scr_AddString("changeclass");
						Game::Scr_Notify(ent, Game::SL_GetString("menuresponse", 0), 2);
					}, 1s);
				}, 1s);
			}, 500ms * (i + 1));
		}
	}

	void Bots::AddMethods()
	{
		Script::AddFunction("SetPing", [](Game::scr_entref_t entref) // gsc: self SetPing(<int>)
		{
			const auto ping = Game::Scr_GetInt(0);

			if (ping < 0 || ping > 999)
			{
				Game::Scr_ParamError(0, "^1SetPing: Ping needs to be between 0 and 999!\n");
				return;
			}

			const auto* gentity = Script::GetEntFromEntRef(entref);
			auto* client = Script::GetClientFromEnt(gentity);

			if (!client->bIsTestClient)
			{
				Game::Scr_Error("^1SetPing: Can only call on a bot!\n");
				return;
			}

			client->ping = static_cast<int16_t>(ping);
		});

		Script::AddFunction("IsBot", [](Game::scr_entref_t entref) // Usage: <bot> IsBot();
		{
			const auto* gentity = Script::GetEntFromEntRef(entref);
			const auto* client = Script::GetClientFromEnt(gentity);

			Game::Scr_AddBool(client->bIsTestClient == 1);
		});

		Script::AddFunction("BotStop", [](Game::scr_entref_t entref) // Usage: <bot> BotStop();
		{
			const auto* gentity = Script::GetEntFromEntRef(entref);
			const auto* client = Script::GetClientFromEnt(gentity);

			if (!client->bIsTestClient)
			{
				Game::Scr_Error("^1BotStop: Can only call on a bot!\n");
				return;
			}

			g_botai[entref.entnum] = {0};
			g_botai[entref.entnum].weapon = 1;
		});

		Script::AddFunction("BotWeapon", [](Game::scr_entref_t entref) // Usage: <bot> BotWeapon(<str>);
		{
			const auto* weapon = Game::Scr_GetString(0);

			const auto* gentity = Script::GetEntFromEntRef(entref);
			const auto* client = Script::GetClientFromEnt(gentity);

			if (!client->bIsTestClient)
			{
				Game::Scr_Error("^1BotWeapon: Can only call on a bot!\n");
				return;
			}

			if (weapon == nullptr || weapon[0] == '\0')
			{
				g_botai[entref.entnum].weapon = 1;
				return;
			}

			const auto weapId = Game::G_GetWeaponIndexForName(weapon);
			g_botai[entref.entnum].weapon = static_cast<uint16_t>(weapId);
		});

		Script::AddFunction("BotAction", [](Game::scr_entref_t entref) // Usage: <bot> BotAction(<str action>);
		{
			const auto* action = Game::Scr_GetString(0);

			if (action == nullptr)
			{
				Game::Scr_ParamError(0, "^1BotAction: Illegal parameter!\n");
				return;
			}

			const auto* gentity = Script::GetEntFromEntRef(entref);
			const auto* client = Script::GetClientFromEnt(gentity);

			if (!client->bIsTestClient)
			{
				Game::Scr_Error("^1BotAction: Can only call on a bot!\n");
				return;
			}

			if (action[0] != '+' && action[0] != '-')
			{
				Game::Scr_ParamError(0, "^1BotAction: Sign for action must be '+' or '-'.\n");
				return;
			}

			for (auto i = 0u; i < std::extent_v<decltype(BotActions)>; ++i)
			{
				if (strcmp(&action[1], BotActions[i].action) != 0)
					continue;

				if (action[0] == '+')
					g_botai[entref.entnum].buttons |= BotActions[i].key;
				else
					g_botai[entref.entnum].buttons &= ~(BotActions[i].key);

				return;
			}

			Game::Scr_ParamError(0, "^1BotAction: Unknown action.\n");
		});

		Script::AddFunction("BotMovement", [](Game::scr_entref_t entref) // Usage: <bot> BotMovement(<int>, <int>);
		{
			auto forwardInt = Game::Scr_GetInt(0);
			auto rightInt = Game::Scr_GetInt(1);

			const auto* gentity = Script::GetEntFromEntRef(entref);
			const auto* client = Script::GetClientFromEnt(gentity);

			if (!client->bIsTestClient)
			{
				Game::Scr_Error("^1BotMovement: Can only call on a bot!\n");
				return;
			}

			forwardInt = std::clamp(forwardInt, -128, 127);
			rightInt = std::clamp(rightInt, -128, 127);

			g_botai[entref.entnum].forward = static_cast<int8_t>(forwardInt);
			g_botai[entref.entnum].right = static_cast<int8_t>(rightInt);
		});
	}

	void Bots::BotAiAction()
	{
		for (auto i = 0; i < *Game::svs_numclients; ++i)
		{
			auto* client = &Game::svs_clients[i];

			if (client->state < Game::CS_CONNECTED)
				continue;

			if (!client->bIsTestClient)
				continue;

			Game::usercmd_s ucmd = {0};

			ucmd.serverTime = *Game::svs_time;

			ucmd.buttons = g_botai[i].buttons;
			ucmd.forwardmove = g_botai[i].forward;
			ucmd.rightmove = g_botai[i].right;
			ucmd.weapon = g_botai[i].weapon;

			client->deltaMessage = client->netchan.outgoingSequence - 1;

			Game::SV_ClientThink(client, &ucmd);
		}
	}

	constexpr auto SV_UpdateBots = 0x626E50;
	__declspec(naked) void Bots::SV_UpdateBots_Hk()
	{
		__asm
		{
			pushad

			call SV_UpdateBots

			// If bot warfare isn't being used let's keep
			// test clients normal functionality
			mov eax, Bots::SVBotWarfare
			cmp byte ptr [eax + 0x10], 0;

			jz skip

			call Bots::BotAiAction

		skip:
			popad
			ret
		}
	}

	Bots::Bots()
	{
		// Replace connect string
		Utils::Hook::Set<const char*>(0x48ADA6, "connect bot%d \"\\cg_predictItems\\1\\cl_anonymous\\0\\color\\4\\head\\default\\model\\multi\\snaps\\20\\rate\\5000\\name\\%s\\protocol\\%d\\checksum\\%d\\statver\\%d %u\\qport\\%d\"");

		// Intercept sprintf for the connect string
		Utils::Hook(0x48ADAB, Bots::BuildConnectString, HOOK_CALL).install()->quick();

		Bots::SVBotWarfare = Game::Dvar_RegisterBool("sv_botWarfare", false,
			Game::DVAR_FLAG_NONE, "Allow bot warfare mod to override default bot behaviour");

		Utils::Hook(0x627021, SV_UpdateBots_Hk, HOOK_CALL).install()->quick();
		Utils::Hook(0x627241, SV_UpdateBots_Hk, HOOK_CALL).install()->quick();

		// Zero the bot command array
		for (auto i = 0u; i < std::extent_v<decltype(g_botai)>; i++)
		{
			g_botai[i] = {0};
			g_botai[i].weapon = 1; // Prevent the bots from defaulting to the 'none' weapon
		}

		Command::Add("spawnBot", [](Command::Params* params)
		{
			auto count = 1u;

			if (params->length() > 1)
			{
				if (params->get(1) == "all"s)
				{
					count = *Game::svs_numclients;
				}
				else
				{
					char* endptr;
					const auto* input = params->get(1);
					count = std::strtoul(input, &endptr, 10);

					if (input == endptr)
					{
						Logger::Print("Warning: %s is not a valid input\n"
							"Usage: %s optional <number of bots> or optional <all>\n",
							input, params->get(0));
					}
				}
			}

			count = std::min(static_cast<unsigned int>(*Game::svs_numclients), count);

			// Check if ingame and host
			if (!Game::SV_Loaded())
			{
				Toast::Show("cardicon_headshot", "^1Error", "You need to be host to spawn bots!", 3000);
				Logger::Print("You need to be host to spawn bots!\n");
				return;
			}

			Toast::Show("cardicon_headshot", "^2Success", Utils::String::VA("Spawning %d %s...", count, (count == 1 ? "bot" : "bots")), 3000);
			Logger::Print("Spawning %d %s...\n", count, (count == 1 ? "bot" : "bots"));

			Bots::Spawn(count);
		});

		Bots::AddMethods();
	}
}
