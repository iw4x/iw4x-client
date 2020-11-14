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
		int8 forward;
		int8 right;
		/* Weapon */
		unsigned short weapon;
	} BotMovementInfo_t;

	BotMovementInfo_t g_botai[MAX_G_BOTAI_ENTRIES];

	struct BotAction_t
	{
		const char* action;
		int key;
	};

	const BotAction_t BotActions[] =
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

	unsigned int Bots::GetClientNum(Game::client_s* cl)
	{
		unsigned int num;

		num = ((byte*)cl - (byte*)Game::svs_clients) / sizeof(Game::client_s);

		return num;
	}

	bool Bots::IsValidClientNum(unsigned int cNum)
	{
		return (cNum >= 0) && (cNum < MAX_G_BOTAI_ENTRIES);
	}

	void Bots::BuildConnectString(char* buffer, const char* connectString, int num, int, int protocol, int checksum, int statVer, int statStuff, int port)
	{
		static int botId = 0;

		if (Bots::BotNames.empty())
		{
			FileSystem::File bots("bots.txt");

			if (bots.exists())
			{
				std::vector<std::string> names = Utils::String::Explode(bots.getBuffer(), '\n');

				for (auto name : names)
				{
					Utils::String::Replace(name, "\r", "");
					name = Utils::String::Trim(name);

					if (!name.empty())
					{
						Bots::BotNames.push_back(name);
					}
				}
			}

			if (Bots::BotNames.empty())
			{
				Bots::BotNames.push_back("bot");
			}
		}

		botId %= Bots::BotNames.size();
		strncpy_s(buffer, 0x400, Utils::String::VA(connectString, num, Bots::BotNames[botId++].data(), protocol, checksum, statVer, statStuff, port), 0x400);
	}

	void Bots::Spawn(unsigned int count)
	{
		for (unsigned int i = 0; i < count; ++i)
		{
			Scheduler::OnDelay([]()
			{
				for (int i = 0; i < 3; ++i)
				{
					Game::gentity_t* entRef = Game::SV_AddTestClient();
					if (entRef)
					{
						Scheduler::OnDelay([entRef]()
						{
							Game::Scr_AddString("autoassign");
							Game::Scr_AddString("team_marinesopfor");
							Game::Scr_Notify(entRef, Game::SL_GetString("menuresponse", 0), 2);

							Scheduler::OnDelay([entRef]()
							{
								Game::Scr_AddString(Utils::String::VA("class%d", Utils::Cryptography::Rand::GenerateInt() % 5));
								Game::Scr_AddString("changeclass");
								Game::Scr_Notify(entRef, Game::SL_GetString("menuresponse", 0), 2);
							}, 1s);
						}, 1s);

						break;
					}
				}
			}, 500ms * (i + 1));
		}
	}

	void Bots::AddMethods()
	{

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

		// zero the bot command array
		for (int i = 0; i < MAX_G_BOTAI_ENTRIES; i++)
		{
			g_botai[i] = { 0 };
			g_botai[i].weapon = 1; // prevent the bots from defaulting to the 'none' weapon
		}

		// have the bots perform the command every server frame
		Scheduler::OnFrame([]()
		{
			if (!Game::SV_Loaded())
				return;

			int time = *Game::svs_time;
			for (int i = 0; i < *Game::svs_numclients; ++i)
			{
				Game::client_t* client = &Game::svs_clients[i];

				if (client->state < 3)
					continue;

				if (!client->isBot)
					continue;

				Game::usercmd_s ucmd = { 0 };

				ucmd.serverTime = time;

				ucmd.buttons = g_botai[i].buttons;
				ucmd.forwardmove = g_botai[i].forward;
				ucmd.rightmove = g_botai[i].right;
				ucmd.weapon = g_botai[i].weapon;

				client->deltaMessage = client->outgoingSequence - 1;

				Game::SV_ClientThink(client, &ucmd);
			}
		});

		Command::Add("spawnBot", [](Command::Params* params)
		{
			unsigned int count = 1;

			if (params->length() > 1)
			{
				if (params->get(1) == "all"s) count = static_cast<unsigned int>(-1);
				else count = atoi(params->get(1));
			}

			count = std::min(18u, count);

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
