#include <STDInclude.hpp>
#include "GSC/Script.hpp"

namespace Components
{
	std::vector<std::string> Bots::BotNames;

	struct BotMovementInfo
	{
		int buttons; // Actions
		int8_t forward;
		int8_t right;
		uint16_t weapon;
		bool active;
	};

	static BotMovementInfo g_botai[18];

	struct BotAction
	{
		std::string action;
		int key;
	};

	static const BotAction BotActions[] =
	{
		{ "gostand", Game::usercmdButtonBits::CMD_BUTTON_UP },
		{ "gocrouch", Game::usercmdButtonBits::CMD_BUTTON_CROUCH },
		{ "goprone", Game::usercmdButtonBits::CMD_BUTTON_PRONE },
		{ "fire", Game::usercmdButtonBits::CMD_BUTTON_ATTACK },
		{ "melee", Game::usercmdButtonBits::CMD_BUTTON_MELEE },
		{ "frag", Game::usercmdButtonBits::CMD_BUTTON_FRAG },
		{ "smoke",  Game::usercmdButtonBits::CMD_BUTTON_OFFHAND_SECONDARY },
		{ "reload", Game::usercmdButtonBits::CMD_BUTTON_RELOAD },
		{ "sprint", Game::usercmdButtonBits::CMD_BUTTON_SPRINT },
		{ "leanleft", Game::usercmdButtonBits::CMD_BUTTON_LEAN_LEFT },
		{ "leanright", Game::usercmdButtonBits::CMD_BUTTON_LEAN_RIGHT },
		{ "ads", Game::usercmdButtonBits::CMD_BUTTON_ADS },
		{ "holdbreath", Game::usercmdButtonBits::CMD_BUTTON_BREATH },
		{ "usereload", Game::usercmdButtonBits::CMD_BUTTON_USE_RELOAD },
		{ "activate", Game::usercmdButtonBits::CMD_BUTTON_ACTIVATE },
	};

	int Bots::BuildConnectString(char* buffer, const char* connectString, int num, int, int protocol, int checksum, int statVer, int statStuff, int port)
	{
		static size_t botId = 0;
		static bool loadedNames = false; // Load file only once
		const char* botName;

		if (Bots::BotNames.empty() && !loadedNames)
		{
			FileSystem::File bots("bots.txt");
			loadedNames = true;

			if (bots.exists())
			{
				auto names = Utils::String::Split(bots.getBuffer(), '\n');

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

		return _snprintf_s(buffer, 0x400, _TRUNCATE, connectString, num, botName, protocol, checksum, statVer, statStuff, port);
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
					Game::Scr_Notify(ent, Game::SL_GetString("menuresponse", 0), 2);

					Scheduler::Once([ent]
					{
						Game::Scr_AddString(Utils::String::VA("class%u", Utils::Cryptography::Rand::GenerateInt() % 5u));
						Game::Scr_AddString("changeclass");
						Game::Scr_Notify(ent, Game::SL_GetString("menuresponse", 0), 2);
					}, Scheduler::Pipeline::SERVER, 1s);

				}, Scheduler::Pipeline::SERVER, 1s);

			}, Scheduler::Pipeline::SERVER, 500ms * (i + 1));
		}
	}

	void Bots::GScr_isTestClient(Game::scr_entref_t entref)
	{
		const auto* ent = Game::GetPlayerEntity(entref);
		Game::Scr_AddBool(Game::SV_IsTestClient(ent->s.number) != 0);
	}

	void Bots::AddMethods()
	{
		Script::AddMethod("IsBot", Bots::GScr_isTestClient); // Usage: self IsBot();
		Script::AddMethod("IsTestClient", Bots::GScr_isTestClient); // Usage: self IsTestClient();

		Script::AddMethod("BotStop", [](Game::scr_entref_t entref) // Usage: <bot> BotStop();
		{
			const auto* ent = Game::GetPlayerEntity(entref);

			if (Game::SV_IsTestClient(ent->s.number) == 0)
			{
				Game::Scr_Error("^1BotStop: Can only call on a bot!\n");
				return;
			}

			g_botai[entref.entnum] = {0};
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

		Game::usercmd_s userCmd = {0};

		userCmd.serverTime = *Game::svs_time;

		userCmd.buttons = g_botai[entnum].buttons;
		userCmd.forwardmove = g_botai[entnum].forward;
		userCmd.rightmove = g_botai[entnum].right;
		userCmd.weapon = g_botai[entnum].weapon;

		Game::SV_ClientThink(cl, &userCmd);
	}

	constexpr auto SV_BotUserMove = 0x626E50;
	__declspec(naked) void Bots::SV_BotUserMove_Hk()
	{
		__asm
		{
			pushad

			push edi
			call Bots::BotAiAction
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
			call Bots::G_SelectWeaponIndex
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
		AssertOffset(Game::client_s, bIsTestClient, 0x41AF0);

		// Replace connect string
		Utils::Hook::Set<const char*>(0x48ADA6, "connect bot%d \"\\cg_predictItems\\1\\cl_anonymous\\0\\color\\4\\head\\default\\model\\multi\\snaps\\20\\rate\\5000\\name\\%s\\protocol\\%d\\checksum\\%d\\statver\\%d %u\\qport\\%d\"");

		// Intercept sprintf for the connect string
		Utils::Hook(0x48ADAB, Bots::BuildConnectString, HOOK_CALL).install()->quick();

		Utils::Hook(0x627021, Bots::SV_BotUserMove_Hk, HOOK_CALL).install()->quick();
		Utils::Hook(0x627241, Bots::SV_BotUserMove_Hk, HOOK_CALL).install()->quick();

		Utils::Hook(0x441B80, Bots::G_SelectWeaponIndex_Hk, HOOK_JUMP).install()->quick();

		// Reset BotMovementInfo.active when client is dropped
		Events::OnClientDisconnect([](const int clientNum)
		{
			g_botai[clientNum].active = false;
		});

		// Zero the bot command array
		for (std::size_t i = 0; i < std::extent_v<decltype(g_botai)>; ++i)
		{
			std::memset(&g_botai[i], 0, sizeof(BotMovementInfo));
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

			Bots::Spawn(count);
		});

		Bots::AddMethods();

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
