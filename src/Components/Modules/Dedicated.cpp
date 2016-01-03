#include "..\..\STDInclude.hpp"

namespace Components
{
	Dvar::Var Dedicated::Dedi;
	std::vector<Dedicated::Callback> Dedicated::FrameCallbacks;

	bool Dedicated::IsDedicated()
	{
		return (Dedicated::Dedi.Get<int>() != 0);
	}

	void Dedicated::InitDedicatedServer()
	{
		const char* fastfiles[7] =
		{
			"code_post_gfx_mp",
			"localized_code_post_gfx_mp",
			"ui_mp",
			"localized_ui_mp",
			"common_mp",
			"localized_common_mp",
			"patch_mp"
		};

		memcpy((void*)0x66E1CB0, &fastfiles, sizeof(fastfiles));
		Game::LoadInitialFF();

		Utils::Hook::Call<void()>(0x4F84C0)();
	}

	void Dedicated::PostInitialization()
	{
		Command::Execute("exec autoexec.cfg");
		Command::Execute("onlinegame 1");
		Command::Execute("exec default_xboxlive.cfg");
		Command::Execute("xblive_rankedmatch 1");
		Command::Execute("xblive_privatematch 1");
		Command::Execute("xstartprivatematch");
		//Command::Execute("xstartlobby");
		Command::Execute("sv_network_fps 1000");
		Command::Execute("com_maxfps 125");

		// Process command line?
		//Utils::Hook::Call<void()>(0x60C3D0)();
	}

	void __declspec(naked) Dedicated::PostInitializationStub()
	{
		__asm
		{
			call Dedicated::PostInitialization

			// Start Com_EvenLoop
			mov eax, 43D140h
			jmp eax
		}
	}

	void Dedicated::MapRotate()
	{
		if (Dvar::Var("party_enable").Get<bool>() && Dvar::Var("party_host").Get<bool>())
		{
			Logger::Print("Not performing map rotation as we are hosting a party!\n");
			return;
		}

		Logger::Print("Rotating map...\n");

		// if nothing, just restart
		if (!Dvar::Var("sv_mapRotation").Get<std::string>().size())
		{
			Logger::Print("No rotation defined, restarting map.\n");
			Command::Execute(Utils::VA("map %s", Dvar::Var("mapname").Get<const char*>()), true);
			return;
		}

		// first, check if the string contains nothing
		if (!Dvar::Var("sv_mapRotationCurrent").Get<std::string>().size())
		{
			Logger::Print("Current map rotation has finished, reloading...\n");
			Dvar::Var("sv_mapRotationCurrent").Set(Dvar::Var("sv_mapRotation").Get<const char*>());
		}

		std::string rotation = Dvar::Var("sv_mapRotationCurrent").Get<std::string>();

		// Ignores " for now, too lazy to implement
		// TODO: Implement!
		auto tokens = Utils::Explode(rotation, ' ');

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
				for (unsigned int j = (i + 2); j < tokens.size(); j++)
				{
					if (j != (i + 2)) rotation += " ";
					rotation += tokens[j];
				}

				Dvar::Var("sv_mapRotationCurrent").Set(rotation);

				Logger::Print("Loading new map: %s\n", value.data());
				Command::Execute(Utils::VA("map %s", value.data()), true);
				break;
			}
			else if (key == "gametype")
			{
				Logger::Print("Applying new gametype: %s\n", value.data());
				Dvar::Var("g_gametype").Set(value);
			}
			else if (key == "fs_game")
			{
				Logger::Print("Applying new mod: %s\n", value.data());
				Dvar::Var("fs_game").Set(value);
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

		Network::Address master(Utils::VA("%s:%u", masterServerName, masterPort));

		Logger::Print("Sending heartbeat to master: %s:%u\n", masterServerName, masterPort);

		Network::Send(master, Utils::VA("heartbeat %s\n", "IW4"));
	}

	void Dedicated::OnFrame(Dedicated::Callback callback)
	{
		Dedicated::FrameCallbacks.push_back(callback);
	}

	void Dedicated::FrameStub()
	{
		for (auto callback : Dedicated::FrameCallbacks)
		{
			callback();
		}

		Utils::Hook::Call<void()>(0x5A8E80)();
	}

	Dedicated::Dedicated()
	{
		Dedicated::Dedi = Dvar::Register<int>("dedicated", 0, 0, 2, Game::dvar_flag::DVAR_FLAG_SERVERINFO | Game::dvar_flag::DVAR_FLAG_WRITEPROTECTED, "Start as dedicated");

		// TODO: Beautify!
		char* cmd = GetCommandLineA();
		char* value = strstr(cmd, " dedicated");

		if (value)
		{
			value += 10;

			while (*value == ' ' || *value == '"')
				value++;

			char num[2] = { 0, 0 };
			num[0] = *value;

			int dediVal = atoi(num);

			if (dediVal && dediVal < 3)
			{
				Dedicated::Dedi.SetRaw(dediVal);
			}
		}

		if (Dedicated::IsDedicated())
		{
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

			// Map rotation
			Utils::Hook::Set(0x4152E8, Dedicated::MapRotate);

			// Dedicated frame handler
			Utils::Hook(0x4B0F81, Dedicated::FrameStub, HOOK_CALL).Install()->Quick();

			// Post initialization point
			Utils::Hook(0x60BFBF, Dedicated::PostInitializationStub, HOOK_JUMP).Install()->Quick();

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
		}
	}

	Dedicated::~Dedicated()
	{
		Dedicated::FrameCallbacks.clear();
	}
}
