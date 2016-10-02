#include "STDInclude.hpp"

namespace Components
{
	wink::signal<wink::slot<Dedicated::Callback>> Dedicated::FrameSignal;
	wink::signal<wink::slot<Dedicated::Callback>> Dedicated::FrameOnceSignal;

	bool Dedicated::SendChat;

	bool Dedicated::IsEnabled()
	{
		return Flags::HasFlag("dedicated");
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
		Command::Execute("com_maxfps 125");

		// Process command line?
		Utils::Hook::Call<void()>(0x60C3D0)();
	}

	__declspec(naked) void Dedicated::PostInitializationStub()
	{
		__asm
		{
			call Dedicated::PostInitialization

			// Start Com_EvenLoop
			mov eax, 43D140h
			jmp eax
		}
	}

	const char* Dedicated::EvaluateSay(char* text)
	{
		Dedicated::SendChat = true;

		if (text[1] == '/')
		{
			Dedicated::SendChat = false;
			text[1] = text[0];
			++text;
		}

		return text;
	}

	__declspec(naked) void Dedicated::PreSayStub()
	{
		__asm
		{
			mov eax, [esp + 100h + 10h]

			push eax
			call EvaluateSay
			add esp, 4h

			mov [esp + 100h + 10h], eax

			jmp Colors::CleanStrStub
		}
	}

	__declspec(naked) void Dedicated::PostSayStub()
	{
		__asm
		{
			// eax is used by the callee
			push eax

			xor eax, eax
			mov al, Dedicated::SendChat

			test al, al
			jnz return

			// Don't send the chat
			pop eax
			retn

		return:
			pop eax

			// Jump to the target
			push 5DF620h
			retn
		}
	}

	void Dedicated::MapRotate()
	{
		if (!Dedicated::IsEnabled() && Dvar::Var("sv_dontrotate").Get<bool>())
		{
			Dvar::Var("sv_dontrotate").SetRaw(0);
			return;
		}

		if (Dvar::Var("party_enable").Get<bool>() && Dvar::Var("party_host").Get<bool>())
		{
			Logger::Print("Not performing map rotation as we are hosting a party!\n");
			return;
		}

		Logger::Print("Rotating map...\n");

		// if nothing, just restart
		if (Dvar::Var("sv_mapRotation").Get<std::string>().empty())
		{
			Logger::Print("No rotation defined, restarting map.\n");

			if (!Dvar::Var("sv_cheats").Get<bool>())
			{
				Command::Execute(fmt::sprintf("map %s", Dvar::Var("mapname").Get<const char*>()), true);
			}
			else
			{
				Command::Execute(fmt::sprintf("devmap %s", Dvar::Var("mapname").Get<const char*>()), true);
			}

			return;
		}

		// first, check if the string contains nothing
		if (Dvar::Var("sv_mapRotationCurrent").Get<std::string>().empty())
		{
			Logger::Print("Current map rotation has finished, reloading...\n");
			Dvar::Var("sv_mapRotationCurrent").Set(Dvar::Var("sv_mapRotation").Get<const char*>());
		}

		std::string rotation = Dvar::Var("sv_mapRotationCurrent").Get<std::string>();

		auto tokens = Utils::String::Explode(rotation, ' ');

		for (unsigned int i = 0; i < (tokens.size() - 1); i += 2)
		{
			if (i + 1 >= tokens.size())
			{
				Dvar::Var("sv_mapRotationCurrent").Set("");
				Command::Execute("map_rotate", true);
				return;
			}

			std::string key = tokens[i];
			std::string value = tokens[i + 1];

			if (key == "map")
			{
				// Rebuild map rotation string
				rotation.clear();
				for (unsigned int j = (i + 2); j < tokens.size(); ++j)
				{
					if (j != (i + 2)) rotation += " ";
					rotation += tokens[j];
				}

				Dvar::Var("sv_mapRotationCurrent").Set(rotation);

				Logger::Print("Loading new map: %s\n", value.data());
				Command::Execute(fmt::sprintf("map %s", value.data()), true);
				break;
			}
			else if (key == "gametype")
			{
				Logger::Print("Applying new gametype: %s\n", value.data());
				Dvar::Var("g_gametype").Set(value);
			}
			else
			{
				Logger::Print("Unsupported maprotation key '%s', motherfucker!\n", key.data());
			}
		}
	}

	void Dedicated::Heartbeat()
	{
		int masterPort = Dvar::Var("masterPort").Get<int>();
		const char* masterServerName = Dvar::Var("masterServerName").Get<const char*>();

		Network::Address master(fmt::sprintf("%s:%u", masterServerName, masterPort));

		Logger::Print("Sending heartbeat to master: %s:%u\n", masterServerName, masterPort);
		Network::SendCommand(master, "heartbeat", "IW4");
	}

	void Dedicated::Once(Dedicated::Callback* callback)
	{
		Dedicated::FrameOnceSignal.connect(callback);
	}

	void Dedicated::OnFrame(Dedicated::Callback* callback)
	{
		Dedicated::FrameSignal.connect(callback);
	}

	void Dedicated::FrameStub()
	{
		auto copy = Dedicated::FrameSignal;
		copy();

		copy = Dedicated::FrameOnceSignal;
		Dedicated::FrameOnceSignal.clear();
		copy();

		Utils::Hook::Call<void()>(0x5A8E80)();
	}

	Dedicated::Dedicated()
	{
		// Map rotation
		Utils::Hook::Set(0x4152E8, Dedicated::MapRotate);
		Dvar::Register<bool>("sv_dontrotate", false, Game::dvar_flag::DVAR_FLAG_CHEAT, "");

		if (Dedicated::IsEnabled() || ZoneBuilder::IsEnabled()) // Run zonebuilder as dedi :P
		{
			Dvar::Register<bool>("sv_lanOnly", false, Game::dvar_flag::DVAR_FLAG_NONE, "Don't act as node");

			Utils::Hook(0x60BE98, Dedicated::InitDedicatedServer, HOOK_CALL).Install()->Quick();

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
			Utils::Hook::Nop(0x414E4D, 6);          // unknown check in SV_ExecuteClientMessage (0x20F0890 == 0, related to client->f_40)
			Utils::Hook::Nop(0x4DCEE9, 5);          // some deinit renderer function
			Utils::Hook::Nop(0x59A896, 5);          // warning message on a removed subsystem
			Utils::Hook::Nop(0x4B4EEF, 5);          // same as above
			Utils::Hook::Nop(0x64CF77, 5);          // function detecting video card, causes Direct3DCreate9 to be called
			Utils::Hook::Nop(0x60BC52, 0x15);       // recommended settings check

			Utils::Hook::Nop(0x45148B, 5);          // Disable splash screen

			// isHost script call return 0
			Utils::Hook::Set<DWORD>(0x5DEC04, 0);

			// sv_network_fps max 1000, and uncheat
			Utils::Hook::Set<BYTE>(0x4D3C67, 0); // ?
			Utils::Hook::Set<DWORD>(0x4D3C69, 1000);

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

			// Dedicated frame handler
			Utils::Hook(0x4B0F81, Dedicated::FrameStub, HOOK_CALL).Install()->Quick();

			// Intercept chat sending
			Utils::Hook(0x4D000B, Dedicated::PreSayStub, HOOK_CALL).Install()->Quick();
			Utils::Hook(0x4D00D4, Dedicated::PostSayStub, HOOK_CALL).Install()->Quick();
			Utils::Hook(0x4D0110, Dedicated::PostSayStub, HOOK_CALL).Install()->Quick();

			if (!ZoneBuilder::IsEnabled())
			{
				// Post initialization point
				Utils::Hook(0x60BFBF, Dedicated::PostInitializationStub, HOOK_JUMP).Install()->Quick();

#ifdef USE_LEGACY_SERVER_LIST

				// Heartbeats
				Dedicated::OnFrame([] ()
				{
					static int LastHeartbeat = 0;

					if (Dvar::Var("sv_maxclients").Get<int>() > 0 && !LastHeartbeat || (Game::Com_Milliseconds() - LastHeartbeat) > 120 * 1000)
					{
						LastHeartbeat = Game::Com_Milliseconds();
						Dedicated::Heartbeat();
					}
				});
#endif
			}

			Dvar::OnInit([] ()
			{
				Dvar::Register<const char*>("sv_sayName", "^7Console", Game::dvar_flag::DVAR_FLAG_NONE, "The name to pose as for 'say' commands");

				// Say command
				Command::AddSV("say", [] (Command::Params params)
				{
					if (params.Length() < 2) return;

					std::string message = params.Join(1);
					std::string name = Dvar::Var("sv_sayName").Get<std::string>();

					if (!name.empty())
					{
						Game::SV_GameSendServerCommand(-1, 0, Utils::String::VA("%c \"%s: %s\"", 104, name.data(), message.data()));
						Game::Com_Printf(15, "%s: %s\n", name.data(), message.data());
					}
					else
					{
						Game::SV_GameSendServerCommand(-1, 0, Utils::String::VA("%c \"Console: %s\"", 104, message.data()));
						Game::Com_Printf(15, "Console: %s\n", message.data());
					}
				});

				// Tell command
				Command::AddSV("tell", [] (Command::Params params)
				{
					if (params.Length() < 3) return;

					int client = atoi(params[1]);
					std::string message = params.Join(2);
					std::string name = Dvar::Var("sv_sayName").Get<std::string>();

					if (!name.empty())
					{
						Game::SV_GameSendServerCommand(client, 0, Utils::String::VA("%c \"%s: %s\"", 104, name.data(), message.data()));
						Game::Com_Printf(15, "%s -> %i: %s\n", name.data(), client, message.data());
					}
					else
					{
						Game::SV_GameSendServerCommand(client, 0, Utils::String::VA("%c \"Console: %s\"", 104, message.data()));
						Game::Com_Printf(15, "Console -> %i: %s\n", client, message.data());
					}
				});

				// Sayraw command
				Command::AddSV("sayraw", [] (Command::Params params)
				{
					if (params.Length() < 2) return;

					std::string message = params.Join(1);
					Game::SV_GameSendServerCommand(-1, 0, Utils::String::VA("%c \"%s\"", 104, message.data()));
					Game::Com_Printf(15, "Raw: %s\n", message.data());
				});

				// Tellraw command
				Command::AddSV("tellraw", [] (Command::Params params)
				{
					if (params.Length() < 3) return;

					int client = atoi(params[1]);
					std::string message = params.Join(2);
					Game::SV_GameSendServerCommand(client, 0, Utils::String::VA("%c \"%s\"", 104, message.data()));
					Game::Com_Printf(15, "Raw -> %i: %s\n", client, message.data());
				});

				// ! command
				Command::AddSV("!", [] (Command::Params params)
				{
					if (params.Length() != 2) return;

					int client = -1;
					if (params[1] != "all"s)
					{
						client = atoi(params[1]);

						if (client >= *reinterpret_cast<int*>(0x31D938C))
						{
							Game::Com_Printf(0, "Invalid player.\n");
							return;
						}
					}

					Game::SV_GameSendServerCommand(client, 0, Utils::String::VA("%c \"\"", 106));
				});
			});
		}
	}

	Dedicated::~Dedicated()
	{
		Dedicated::FrameOnceSignal.clear();
		Dedicated::FrameSignal.clear();
	}
}
