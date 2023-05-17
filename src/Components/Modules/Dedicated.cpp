#include <STDInclude.hpp>
#include <Utils/InfoString.hpp>

#include "CardTitles.hpp"
#include "ClanTags.hpp"
#include "Events.hpp"
#include "Party.hpp"
#include "ServerCommands.hpp"

namespace Components
{
	SteamID Dedicated::PlayerGuids[18][2];

	Dvar::Var Dedicated::SVLanOnly;
	Dvar::Var Dedicated::SVMOTD;
	Dvar::Var Dedicated::COMLogFilter;

	const Game::dvar_t* Dedicated::com_dedicated;

	bool Dedicated::IsEnabled()
	{
		static std::optional<bool> flag;

		if (!flag.has_value())
		{
			flag.emplace(Flags::HasFlag("dedicated"));
		}

		return flag.value();
	}

	bool Dedicated::IsRunning()
	{
		assert(*Game::com_sv_running);
		return *Game::com_sv_running && (*Game::com_sv_running)->current.enabled;
	}

	void Dedicated::InitDedicatedServer()
	{
		static const char* fastfiles[7] =
		{
			"code_post_gfx_mp",
			"localized_code_post_gfx_mp",
			"ui_mp",
			"localized_ui_mp",
			"common_mp",
			"localized_common_mp",
			"patch_mp"
		};

		std::memcpy(reinterpret_cast<void*>(0x66E1CB0), &fastfiles, sizeof(fastfiles));
		Game::R_LoadGraphicsAssets();

		if (COMLogFilter.get<bool>())
		{
			Utils::Hook::Nop(0x647466, 5); // 'dvar set' lines
			Utils::Hook::Nop(0x5DF4F2, 5); // 'sending splash open' lines
		}

		Utils::Hook::Call<void()>(0x4F84C0)();
	}

	void Dedicated::PostInitialization()
	{
		Command::Execute("exec autoexec.cfg");
		Command::Execute("onlinegame 1");
		Command::Execute("exec default_xboxlive.cfg");
		Command::Execute("xblive_rankedmatch 1");
		Command::Execute("xblive_privatematch 1");
		Command::Execute("xblive_privateserver 0");
		Command::Execute("xstartprivatematch");
		//Command::Execute("xstartlobby");
		Command::Execute("sv_network_fps 1000");
		Command::Execute("cl_maxpackets 125");
		Command::Execute("snaps 30");
		Command::Execute("com_maxfps 125");

		Game::Com_AddStartupCommands();
	}

	__declspec(naked) void Dedicated::PostInitializationStub()
	{
		__asm
		{
			pushad
			call PostInitialization
			popad

			// Start Com_EvenLoop
			mov eax, 43D140h
			jmp eax
		}
	}

	void Dedicated::Com_ClampMsec(const int msec)
	{
		if (msec > 500 && msec < 500000)
		{
			Game::Com_PrintWarning(Game::CON_CHANNEL_SYSTEM, "Hitch warning: %i msec frame time\n", msec);
		}
	}

	__declspec(naked) void Dedicated::Com_ClampMsec_Stub()
	{
		using namespace Game;

		__asm
		{
			pushad

			push ecx
			call Com_ClampMsec
			add esp, 0x4

			popad

			// Game's code
			mov edx, dword ptr com_sv_running

			push 0x47DDB8
			ret
		}
	}

	void Dedicated::TransmitGuids()
	{
		std::string list = Utils::String::VA("%c", 20);

		for (std::size_t i = 0; i < Game::MAX_CLIENTS; ++i)
		{
			if (Game::svs_clients[i].header.state >= 3)
			{
				list.append(Utils::String::VA(" %llX", Game::svs_clients[i].steamID));

				Utils::InfoString info(Game::svs_clients[i].userinfo);
				list.append(Utils::String::VA(" %llX", std::strtoull(info.get("realsteamId").data(), nullptr, 16)));
			}
			else
			{
				list.append(" 0 0");
			}
		}

		Game::SV_GameSendServerCommand(-1, Game::SV_CMD_CAN_IGNORE, list.data());
	}

	void Dedicated::TimeWrapStub(Game::errorParm_t code, const char* message)
	{
		Scheduler::Once([]
		{
			std::string mapname = (*Game::sv_mapname)->current.string;

			if (!Party::IsEnabled()) // Time wrapping should not occur in party servers, but yeah...
			{
				if (mapname.empty()) mapname = "mp_rust";
				Command::Execute(std::format("map {}", mapname), true);
			}
		}, Scheduler::Pipeline::SERVER);

		Game::Com_Error(code, message);
	}

	void Dedicated::Heartbeat()
	{	
		// Do not send a heartbeat if sv_lanOnly is set to true
		if (SVLanOnly.get<bool>())
		{
			return;
		}

		const auto masterPort = (*Game::com_masterPort)->current.integer;
		const auto* masterServerName = (*Game::com_masterServerName)->current.string;

		Network::Address master(Utils::String::VA("%s:%u", masterServerName, masterPort));

		Logger::Print("Sending heartbeat to master: {}:{}\n", masterServerName, masterPort);
		Network::SendCommand(master, "heartbeat", "IW4");
	}

	Dedicated::Dedicated()
	{
		COMLogFilter = Dvar::Register<bool>("com_logFilter", true,
			Game::DVAR_LATCH, "Removes ~95% of unneeded lines from the log");

		if (IsEnabled() || ZoneBuilder::IsEnabled())
		{
			// Make sure all callbacks are handled
			Scheduler::Loop(Steam::SteamAPI_RunCallbacks, Scheduler::Pipeline::SERVER);

			Utils::Hook(0x60BE98, InitDedicatedServer, HOOK_CALL).install()->quick();

			Utils::Hook::Set<BYTE>(0x683370, 0xC3); // steam sometimes doesn't like the server

			Utils::Hook::Set<BYTE>(0x5B4FF0, 0xC3); // self-registration on party
			Utils::Hook::Set<BYTE>(0x426130, 0xC3); // other party stuff?

			Utils::Hook::Set<BYTE>(0x4D7030, 0xC3); // upnp stuff

			Utils::Hook::Set<BYTE>(0x4B0FC3, 0x04); // make CL_Frame do client packets, even for game state 9
			Utils::Hook::Set<BYTE>(0x4F5090, 0xC3); // init sound system (1)
			Utils::Hook::Set<BYTE>(0x507B80, 0xC3); // start render thread
			Utils::Hook::Set<BYTE>(0x4F84C0, 0xC3); // R_Init caller
			Utils::Hook::Set<BYTE>(0x46A630, 0xC3); // init sound system (2)
			Utils::Hook::Set<BYTE>(0x41FDE0, 0xC3); // Com_Frame audio processor?
			Utils::Hook::Set<BYTE>(0x41B9F0, 0xC3); // called from Com_Frame, seems to do renderer stuff
			Utils::Hook::Set<BYTE>(0x41D010, 0xC3); // CL_CheckForResend, which tries to connect to the local server constantly
			Utils::Hook::Set<BYTE>(0x62B6C0, 0xC3); // UI expression 'DebugPrint', mainly to prevent some console spam

			Utils::Hook::Set<BYTE>(0x468960, 0xC3); // some mixer-related function called on shutdown
			Utils::Hook::Set<BYTE>(0x60AD90, 0);    // masterServerName flags

			Utils::Hook::Nop(0x4DCEC9, 2);          // some check preventing proper game functioning
			Utils::Hook::Nop(0x507C79, 6);          // another similar bsp check
			Utils::Hook::Nop(0x414E4D, 6);          // cl->messageAcknowledge > cl->gamestateMessageNum check in SV_ExecuteClientMessage
			Utils::Hook::Nop(0x4DCEE9, 5);          // some deinit renderer function
			Utils::Hook::Nop(0x59A896, 5);          // warning message on a removed subsystem
			Utils::Hook::Nop(0x4B4EEF, 5);          // same as above
			Utils::Hook::Nop(0x64CF77, 5);          // function detecting video card, causes Direct3DCreate9 to be called
			Utils::Hook::Nop(0x60BC52, 0x15);       // recommended settings check

			Utils::Hook::Nop(0x45148B, 5);          // Disable splash screen

			// do not trigger host migration, even if the server is a 'bad host'
			Utils::Hook::Set<BYTE>(0x626AA8, 0xEB);

			// isHost script call return 0
			Utils::Hook::Set<DWORD>(0x5DEC04, 0);

			// r_loadForRenderer default to 0
			Utils::Hook::Set<BYTE>(0x519DDF, 0);

			// disable cheat protection on onlinegame
			Utils::Hook::Set<BYTE>(0x404CF7, 0x80);

			// some d3d9 call on error
			Utils::Hook::Set<BYTE>(0x508470, 0xC3);

			// stop saving a config_mp.cfg
			Utils::Hook::Set<BYTE>(0x60B240, 0xC3);

			// don't load the config
			Utils::Hook::Set<BYTE>(0x4B4D19, 0xEB);

			// Intercept time wrapping
			Utils::Hook(0x62737D, TimeWrapStub, HOOK_CALL).install()->quick();
			//Utils::Hook::Set<DWORD>(0x62735C, 50'000); // Time wrap after 50 seconds (for testing - i don't want to wait 3 weeks)

			if (!ZoneBuilder::IsEnabled())
			{
				Events::OnDvarInit([]
				{
					SVMOTD = Dvar::Register<const char*>("sv_motd", "", Game::DVAR_NONE, "A custom message of the day for servers");
					SVLanOnly = Dvar::Register<bool>("sv_lanOnly", false, Game::DVAR_NONE, "Don't act as node");

					static const char* g_dedicatedEnumNames[] =
					{
						"listen server",
						"dedicated LAN server",
						"dedicated internet server",
						nullptr,
					};

					// IW5MP Dedicated Server adds another flag. That flag should not exist on this version of IW4
					com_dedicated = Game::Dvar_RegisterEnum("dedicated", g_dedicatedEnumNames, 2, Game::DVAR_ROM, "True if this is a dedicated server");
					// Dedicated only behaviour from IW5MP Dedicated Server.
					if (com_dedicated->current.integer != 1 && com_dedicated->current.integer != 2)
					{
						Game::DvarValue value;
						value.integer = 0;
						Game::Dvar_SetVariant(const_cast<Game::dvar_t*>(com_dedicated), value, Game::DVAR_SOURCE_INTERNAL);
					}
				});

				// Post initialization point
				Utils::Hook(0x60BFBF, PostInitializationStub, HOOK_JUMP).install()->quick();

				Utils::Hook(0x47DDB2, Com_ClampMsec_Stub, HOOK_JUMP).install()->quick(); // Com_Frame_Try_Block_Function
				Utils::Hook::Nop(0x47DDB2 + 5, 1);

				// Transmit custom data
				Scheduler::Loop([]
				{
					CardTitles::SendCustomTitlesToClients();
					ClanTags::SendClanTagsToClients();
				}, Scheduler::Pipeline::SERVER, 10s);

				// Heartbeats
				Scheduler::Once(Heartbeat, Scheduler::Pipeline::SERVER);
				Scheduler::Loop(Heartbeat, Scheduler::Pipeline::SERVER, 2min);
			}
		}
		else
		{
			ZeroMemory(PlayerGuids, sizeof(PlayerGuids));

			// Intercept server commands
			ServerCommands::OnCommand(20, [](const Command::Params* params)
			{
				for (int client = 0; client < 18; client++)
				{
					PlayerGuids[client][0].bits = std::strtoull(params->get(2 * client + 1), nullptr, 16);
					PlayerGuids[client][1].bits = std::strtoull(params->get(2 * client + 2), nullptr, 16);

					if (Steam::Proxy::SteamFriends && PlayerGuids[client][1].bits != 0)
					{
						Steam::Proxy::SteamFriends->SetPlayedWith(PlayerGuids[client][1]);
					}
				}

				return true;
			});
		}

		Scheduler::Loop([]
		{
			if (IsRunning())
			{
				TransmitGuids();
			}
		}, Scheduler::Pipeline::SERVER, 15s);
	}
}
