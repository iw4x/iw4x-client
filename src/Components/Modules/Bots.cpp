#include "STDInclude.hpp"

#define KEY_MASK_FIRE           1
#define KEY_MASK_SPRINT         2
#define KEY_MASK_MELEE          4
#define KEY_MASK_RELOAD         16
#define KEY_MASK_LEANLEFT       64
#define KEY_MASK_LEANRIGHT      128
#define KEY_MASK_PRONE          256
#define KEY_MASK_CROUCH         512
#define KEY_MASK_JUMP           1024
#define KEY_MASK_ADS_MODE       2048
#define KEY_MASK_TEMP_ACTION    4096
#define KEY_MASK_HOLDBREATH     8192
#define KEY_MASK_FRAG           16384
#define KEY_MASK_SMOKE          32768
#define KEY_MASK_NIGHTVISION    262144
#define KEY_MASK_ADS            524288
#define KEY_MASK_USE            0x28

#define MAX_G_BOTAI_ENTRIES     18

namespace Components
{
	std::vector<std::string> Bots::BotNames;

	typedef struct BotMovementInfo_t
	{
		/* Actions */
		int buttons;
		/* Movement */
		int8_t forward;
		int8_t right;
		/* Weapon */
		uint16_t weapon;
	} BotMovementInfo_t;

	static BotMovementInfo_t g_botai[MAX_G_BOTAI_ENTRIES];

	struct BotAction_t
	{
		const char* action;
		int key;
	};

	static const BotAction_t BotActions[] =
	{
		{ "gostand",    KEY_MASK_JUMP       },
		{ "gocrouch",   KEY_MASK_CROUCH     },
		{ "goprone",    KEY_MASK_PRONE      },
		{ "fire",       KEY_MASK_FIRE       },
		{ "melee",      KEY_MASK_MELEE      },
		{ "frag",       KEY_MASK_FRAG       },
		{ "smoke",      KEY_MASK_SMOKE      },
		{ "reload",     KEY_MASK_RELOAD     },
		{ "sprint",     KEY_MASK_SPRINT     },
		{ "leanleft",   KEY_MASK_LEANLEFT   },
		{ "leanright",  KEY_MASK_LEANRIGHT  },
		{ "ads",        KEY_MASK_ADS_MODE   },
		{ "holdbreath", KEY_MASK_HOLDBREATH },
		{ "use",        KEY_MASK_USE        },
		{ "0",          8                   },
		{ "1",          32                  },
		{ "2",          65536               },
		{ "3",          131072              },
		{ "4",          1048576             },
		{ "5",          2097152             },
		{ "6",          4194304             },
		{ "7",          8388608             },
		{ "8",          16777216            },
		{ "9",          33554432            },
	};

	bool Bots::IsValidClientNum(unsigned int cNum)
	{
		return (cNum >= 0) && (cNum < (unsigned int)*Game::svs_numclients);
	}

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

	void Bots::Spawn(int count)
	{
		for (auto i = 0; i < count; ++i)
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
		Script::AddFunction("SetPing", [](Game::scr_entref_t id) // gsc: self SetPing(<int>)
		{
			if (Game::Scr_GetNumParam() != 1u || Game::Scr_GetType(0) != Game::VAR_INTEGER)
			{
				Game::Scr_Error("^1SetPing: Needs one integer parameter!\n");
				return;
			}

			const auto ping = Game::Scr_GetInt(0);

			if (ping < 0 || ping > 999)
			{
				Game::Scr_Error("^1SetPing: Ping needs to between 0 and 999!\n");
				return;
			}

			const auto* gentity = Script::GetEntFromEntRef(id);
			auto* client = Script::GetClientFromEnt(gentity);

			if (!client->isBot)
			{
				Game::Scr_Error("^1SetPing: Can only call on a bot!\n");
				return;
			}

			client->ping = static_cast<int16_t>(ping);
		});

		Script::AddFunction("IsBot", [](Game::scr_entref_t id) // Usage: <bot> IsBot();
		{
			const auto* gentity = Script::GetEntFromEntRef(id);
			const auto* client = Script::GetClientFromEnt(gentity);

			Game::Scr_AddInt(client->isBot);
		});

		Script::AddFunction("BotStop", [](Game::scr_entref_t id) // Usage: <bot> BotStop();
		{
			const auto* gentity = Script::GetEntFromEntRef(id);
			const auto* client = Script::GetClientFromEnt(gentity);

			if (!client->isBot)
			{
				Game::Scr_Error("^1BotStop: Can only call on a bot!\n");
				return;
			}

			g_botai[gentity->s.number] = {0};
			g_botai[gentity->s.number].weapon = 1;
		});

		Script::AddFunction("BotWeapon", [](Game::scr_entref_t id) // Usage: <bot> BotWeapon(<str>);
		{
			if (Game::Scr_GetNumParam() != 1u || Game::Scr_GetType(0) != Game::VAR_STRING)
			{
				Game::Scr_Error("^1BotWeapon: Needs one string parameter!\n");
				return;
			}

			const auto* weapon = Game::Scr_GetString(0);

			const auto* gentity = Script::GetEntFromEntRef(id);
			const auto* client = Script::GetClientFromEnt(gentity);

			if (!client->isBot)
			{
				Game::Scr_Error("^1BotWeapon: Can only call on a bot!\n");
				return;
			}

			if (weapon[0] == '\0')
			{
				g_botai[gentity->s.number].weapon = 1;
				return;
			}

			const auto weapId = Game::G_GetWeaponIndexForName(weapon);
			g_botai[gentity->s.number].weapon = static_cast<uint16_t>(weapId);
		});

		Script::AddFunction("BotAction", [](Game::scr_entref_t id) // Usage: <bot> BotAction(<str action>);
		{
			if (Game::Scr_GetNumParam() != 1u || Game::Scr_GetType(0) != Game::VAR_STRING)
			{
				Game::Scr_Error("^1BotAction: Needs one string parameter!\n");
				return;
			}

			const auto* action = Game::Scr_GetString(0);

			const auto* gentity = Script::GetEntFromEntRef(id);
			const auto* client = Script::GetClientFromEnt(gentity);

			if (!client->isBot)
			{
				Game::Scr_Error("^1BotAction: Can only call on a bot!\n");
				return;
			}

			if (action[0] != '+' && action[0] != '-')
			{
				Game::Scr_Error("^1BotAction: Sign for action must be '+' or '-'.\n");
				return;
			}

			for (auto i = 0u; i < sizeof(BotActions) / sizeof(BotAction_t); ++i)
			{
				if (strcmp(&action[1], BotActions[i].action))
					continue;

				if (action[0] == '+')
					g_botai[gentity->s.number].buttons |= BotActions[i].key;
				else
					g_botai[gentity->s.number].buttons &= ~(BotActions[i].key);

				return;
			}

			Game::Scr_Error("^1BotAction: Unknown action.\n");
		});

		Script::AddFunction("BotMovement", [](Game::scr_entref_t id) // Usage: <bot> BotMovement(<int>, <int>);
		{
			if (Game::Scr_GetNumParam() != 2u || Game::Scr_GetType(0) != Game::VAR_INTEGER || Game::Scr_GetType(1) != Game::VAR_INTEGER)
			{
				Game::Scr_Error("^1BotMovement: Needs two integer parameters!\n");
				return;
			}

			auto forwardInt = Game::Scr_GetInt(0);
			auto rightInt = Game::Scr_GetInt(1);

			const auto* gentity = Script::GetEntFromEntRef(id);
			const auto* client = Script::GetClientFromEnt(gentity);

			if (!client->isBot)
			{
				Game::Scr_Error("^1BotMovement: Can only call on a bot!\n");
				return;
			}

			if (forwardInt > 127)
				forwardInt = 127;

			if (forwardInt < -127)
				forwardInt = -127;

			if (rightInt > 127)
				rightInt = 127;

			if (rightInt < -127)
				rightInt = -127;

			g_botai[gentity->s.number].forward = static_cast<int8_t>(forwardInt);
			g_botai[gentity->s.number].right = static_cast<int8_t>(rightInt);
		});
	}

	Bots::Bots()
	{
		// Replace connect string
		Utils::Hook::Set<const char*>(0x48ADA6, "connect bot%d \"\\cg_predictItems\\1\\cl_anonymous\\0\\color\\4\\head\\default\\model\\multi\\snaps\\20\\rate\\5000\\name\\%s\\protocol\\%d\\checksum\\%d\\statver\\%d %u\\qport\\%d\"");

		// Intercept sprintf for the connect string
		Utils::Hook(0x48ADAB, Bots::BuildConnectString, HOOK_CALL).install()->quick();

		// Stop default behavour of bots spinning and shooting
		Utils::Hook(0x627021, 0x4BB9B0, HOOK_CALL).install()->quick();
		Utils::Hook(0x627241, 0x4BB9B0, HOOK_CALL).install()->quick();

		// Zero the bot command array
		for (auto i = 0; i < MAX_G_BOTAI_ENTRIES; i++)
		{
			g_botai[i] = {0};
			g_botai[i].weapon = 1; // Prevent the bots from defaulting to the 'none' weapon
		}

		// Have the bots perform the command every server frame
		Scheduler::OnFrame([]()
		{
			if (!Game::SV_Loaded())
				return;

			for (auto i = 0; i < *Game::svs_numclients; ++i)
			{
				auto* client = &Game::svs_clients[i];

				if (client->state < 3)
					continue;

				if (!client->isBot)
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
		});

		Command::Add("spawnBot", [](Command::Params* params)
		{
			auto count = 1;

			if (params->length() > 1)
			{
				if (params->get(1) == "all"s)
					count = *Game::svs_numclients;
				else
					count = std::atoi(params->get(1));
			}

			count = std::min(*Game::svs_numclients, count);

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

	Bots::~Bots()
	{
		Bots::BotNames.clear();
	}
}
