#include <STDInclude.hpp>
#include <Utils/InfoString.hpp>

#include "Bots.hpp"
#include "ServerList.hpp"

#include "GSC/Script.hpp"

// From Quake-III
#define	ANGLE2SHORT(x) ((int)((x) * (USHRT_MAX + 1) / 360.0f) & USHRT_MAX)
#define	SHORT2ANGLE(x) ((x)* (360.0f / (USHRT_MAX + 1)))

namespace Components
{
	std::vector<Bots::botData> Bots::BotNames;

	Dvar::Var Bots::SVClanName;

	struct BotMovementInfo
	{
		std::int32_t buttons; // Actions
		std::int8_t forward;
		std::int8_t right;
		std::uint16_t weapon;
		bool active;
	};

	static BotMovementInfo g_botai[18];

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
		{ "ads", Game::CMD_BUTTON_ADS },
		{ "holdbreath", Game::CMD_BUTTON_BREATH },
		{ "usereload", Game::CMD_BUTTON_USE_RELOAD },
		{ "activate", Game::CMD_BUTTON_ACTIVATE },
	};

	void Bots::RandomizeBotNames()
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		std::ranges::shuffle(BotNames, gen);
	}

	void Bots::UpdateBotNames()
	{
		const auto masterPort = (*Game::com_masterPort)->current.integer;
		const auto* masterServerName = (*Game::com_masterServerName)->current.string;

		Game::netadr_t master;
		if (ServerList::GetMasterServer(masterServerName, masterPort, master))
		{
			Logger::Print("Getting bots...\n");
			Network::Send(master, "getbots");
		}
	}

	void Bots::LoadBotNames()
	{
		FileSystem::File bots("bots.txt");

		if (!bots.exists())
		{
			return;
		}

		auto data = Utils::String::Split(bots.getBuffer(), '\n');

		auto i = 0;
		for (auto& entry : data)
		{
			if (i >= 18)
			{
				// Only parse 18 names from the file
				break;
			}

			// Take into account for CR line endings
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
					clanAbbrev = entry.substr(pos + 1);
				}

				entry = entry.substr(0, pos);
			}

			BotNames.emplace_back(std::make_pair(entry, clanAbbrev));
			++i;
		}

		if (i)
		{
			RandomizeBotNames();
		}
	}

	int Bots::BuildConnectString(char* buffer, const char* connectString, int num, int, int protocol, int checksum, int statVer, int statStuff, int port)
	{
		static size_t botId = 0; // Loop over the BotNames vector
		static bool loadedNames = false; // Load file only once
		std::string botName;
		std::string clanName;

		if (!loadedNames)
		{
			loadedNames = true;
			LoadBotNames();
		}

		if (!BotNames.empty())
		{
			botId %= BotNames.size();
			const auto index = botId++;
			botName = BotNames[index].first;
			clanName = BotNames[index].second;
		}
		else
		{
			botName = std::format("bot{}", ++botId);
			clanName = "BOT"s;
		}

		if (const auto svClanName = SVClanName.get<std::string>(); !svClanName.empty())
		{
			clanName = svClanName;
		}

		return _snprintf_s(buffer, 0x400, _TRUNCATE, connectString, num, botName.data(), clanName.data(), protocol, checksum, statVer, statStuff, port);
	}

	void Bots::Spawn(unsigned int count)
	{
		for (std::size_t i = 0; i < count; ++i)
		{
			Scheduler::Once([]
			{
				auto* ent = Game::SV_AddTestClient();
				if (ent == nullptr)
					return;

				Scheduler::Once([ent]
				{
					Game::Scr_AddString("autoassign");
					Game::Scr_AddString("team_marinesopfor");
					Game::Scr_Notify(ent, static_cast<std::uint16_t>(Game::SL_GetString("menuresponse", 0)), 2);

					Scheduler::Once([ent]
					{
						Game::Scr_AddString(Utils::String::VA("class%u", Utils::Cryptography::Rand::GenerateInt() % 5u));
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

	void Bots::AddMethods()
	{
		Script::AddMethMultiple(GScr_isTestClient, false, {"IsTestClient", "IsBot"}); // Usage: self IsTestClient();

		Script::AddMethod("BotStop", [](Game::scr_entref_t entref) // Usage: <bot> BotStop();
		{
			const auto* ent = Game::GetPlayerEntity(entref);

			if (Game::SV_IsTestClient(ent->s.number) == 0)
			{
				Game::Scr_Error("^1BotStop: Can only call on a bot!\n");
				return;
			}

			ZeroMemory(&g_botai[entref.entnum], sizeof(BotMovementInfo));
			g_botai[entref.entnum].weapon = 1;
			g_botai[entref.entnum].active = true;
		});

		Script::AddMethod("BotWeapon", [](Game::scr_entref_t entref) // Usage: <bot> BotWeapon(<str>);
		{
			const auto* ent = Game::GetPlayerEntity(entref);

			if (Game::SV_IsTestClient(ent->s.number) == 0)
			{
				Game::Scr_Error("^1BotWeapon: Can only call on a bot!\n");
				return;
			}

			const auto* weapon = Game::Scr_GetString(0);

			if (weapon == nullptr || weapon[0] == '\0')
			{
				g_botai[entref.entnum].weapon = 1;
				return;
			}

			const auto weapId = Game::G_GetWeaponIndexForName(weapon);
			g_botai[entref.entnum].weapon = static_cast<uint16_t>(weapId);
			g_botai[entref.entnum].active = true;
		});

		Script::AddMethod("BotAction", [](Game::scr_entref_t entref) // Usage: <bot> BotAction(<str action>);
		{
			const auto* ent = Game::GetPlayerEntity(entref);

			if (Game::SV_IsTestClient(ent->s.number) == 0)
			{
				Game::Scr_Error("^1BotAction: Can only call on a bot!\n");
				return;
			}

			const auto* action = Game::Scr_GetString(0);

			if (action == nullptr)
			{
				Game::Scr_ParamError(0, "^1BotAction: Illegal parameter!\n");
				return;
			}

			if (action[0] != '+' && action[0] != '-')
			{
				Game::Scr_ParamError(0, "^1BotAction: Sign for action must be '+' or '-'.\n");
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

			Game::Scr_ParamError(0, "^1BotAction: Unknown action.\n");
		});

		Script::AddMethod("BotMovement", [](Game::scr_entref_t entref) // Usage: <bot> BotMovement(<int>, <int>);
		{
			const auto* ent = Game::GetPlayerEntity(entref);

			if (Game::SV_IsTestClient(ent->s.number) == 0)
			{
				Game::Scr_Error("^1BotMovement: Can only call on a bot!\n");
				return;
			}

			const auto forwardInt = std::clamp<int>(Game::Scr_GetInt(0), std::numeric_limits<char>::min(), std::numeric_limits<char>::max());
			const auto rightInt = std::clamp<int>(Game::Scr_GetInt(1), std::numeric_limits<char>::min(), std::numeric_limits<char>::max());

			g_botai[entref.entnum].forward = static_cast<int8_t>(forwardInt);
			g_botai[entref.entnum].right = static_cast<int8_t>(rightInt);
			g_botai[entref.entnum].active = true;
		});
	}

	void Bots::BotAiAction(Game::client_t* cl)
	{
		if (cl->gentity == nullptr)
			return;

		const auto entnum = cl->gentity->s.number;

		// Keep test client functionality
		if (!g_botai[entnum].active)
		{
			Game::SV_BotUserMove(cl);
			return;
		}

		Game::usercmd_s userCmd;
		ZeroMemory(&userCmd, sizeof(Game::usercmd_s));

		userCmd.serverTime = *Game::svs_time;

		userCmd.buttons = g_botai[entnum].buttons;
		userCmd.forwardmove = g_botai[entnum].forward;
		userCmd.rightmove = g_botai[entnum].right;
		userCmd.weapon = g_botai[entnum].weapon;

		userCmd.angles[0] = ANGLE2SHORT((cl->gentity->client->ps.viewangles[0] - cl->gentity->client->ps.delta_angles[0]));
		userCmd.angles[1] = ANGLE2SHORT((cl->gentity->client->ps.viewangles[1] - cl->gentity->client->ps.delta_angles[1]));
		userCmd.angles[2] = ANGLE2SHORT((cl->gentity->client->ps.viewangles[2] - cl->gentity->client->ps.delta_angles[2]));

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

	void Bots::G_SelectWeaponIndex(int clientNum, int iWeaponIndex)
	{
		if (g_botai[clientNum].active)
		{
			g_botai[clientNum].weapon = static_cast<uint16_t>(iWeaponIndex);
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

	Bots::Bots()
	{
		AssertOffset(Game::client_t, bIsTestClient, 0x41AF0);
		AssertOffset(Game::client_t, ping, 0x212C8);

		// Replace connect string
		Utils::Hook::Set<const char*>(0x48ADA6, "connect bot%d \"\\cg_predictItems\\1\\cl_anonymous\\0\\color\\4\\head\\default\\model\\multi\\snaps\\20\\rate\\5000\\name\\%s\\clanAbbrev\\%s\\protocol\\%d\\checksum\\%d\\statver\\%d %u\\qport\\%d\"");

		// Intercept sprintf for the connect string
		Utils::Hook(0x48ADAB, BuildConnectString, HOOK_CALL).install()->quick();

		Utils::Hook(0x627021, SV_BotUserMove_Hk, HOOK_CALL).install()->quick();
		Utils::Hook(0x627241, SV_BotUserMove_Hk, HOOK_CALL).install()->quick();

		Utils::Hook(0x441B80, G_SelectWeaponIndex_Hk, HOOK_JUMP).install()->quick();

		Events::OnDvarInit([]
		{
			SVClanName = Dvar::Register<const char*>("sv_clanName", "", Game::DVAR_NONE, "The clan name for test clients");
		});

		Scheduler::OnGameInitialized(UpdateBotNames, Scheduler::Pipeline::MAIN);

		Network::OnClientPacket("getbotsResponse", [](const Network::Address& address, const std::string& data)
		{
			const auto masterPort = (*Game::com_masterPort)->current.integer;
			const auto* masterServerName = (*Game::com_masterServerName)->current.string;

			Network::Address master(Utils::String::VA("%s:%u", masterServerName, masterPort));
			if (master == address)
			{
				auto botNames = Utils::String::Split(data, '\n');
				for (const auto& entry : botNames)
				{
					BotNames.emplace_back(std::make_pair(entry, "BOT"));
				}

				RandomizeBotNames();
			}
		});

		// Reset BotMovementInfo.active when client is dropped
		Events::OnClientDisconnect([](const int clientNum)
		{
			g_botai[clientNum].active = false;
		});

		// Zero the bot command array
		for (std::size_t i = 0; i < std::extent_v<decltype(g_botai)>; ++i)
		{
			ZeroMemory(&g_botai[i], sizeof(BotMovementInfo));
			g_botai[i].weapon = 1; // Prevent the bots from defaulting to the 'none' weapon
		}

		Command::Add("spawnBot", [](Command::Params* params)
		{
			auto count = 1u;

			if (params->size() > 1)
			{
				if (params->get(1) == "all"s)
				{
					count = *Game::svs_clientCount;
				}
				else
				{
					char* end;
					const auto* input = params->get(1);
					count = std::strtoul(input, &end, 10);

					if (input == end)
					{
						Logger::Warning(Game::CON_CHANNEL_DONT_FILTER, "{} is not a valid input\nUsage: {} optional <number of bots> or optional <\"all\">\n",
							input, params->get(0));
						return;
					}
				}
			}

			count = std::min(static_cast<unsigned int>(*Game::svs_clientCount), count);

			// Check if ingame and host
			if (!Game::SV_Loaded())
			{
				Toast::Show("cardicon_headshot", "^1Error", "You need to be host to spawn bots!", 3000);
				Logger::Print("You need to be host to spawn bots!\n");
				return;
			}

			Toast::Show("cardicon_headshot", "^2Success", Utils::String::VA("Spawning %d %s...", count, (count == 1 ? "bot" : "bots")), 3000);
			Logger::Debug("Spawning {} {}", count, (count == 1 ? "bot" : "bots"));

			Spawn(count);
		});

		AddMethods();

		// In case a loaded mod didn't call "BotStop" before the VM shutdown
		Events::OnVMShutdown([]
		{
			for (std::size_t i = 0; i < std::extent_v<decltype(g_botai)>; ++i)
			{
				g_botai[i].active = false;
			}
		});
	}
}
