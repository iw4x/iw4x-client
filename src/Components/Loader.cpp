#include "STDInclude.hpp"

namespace Components
{
	bool Loader::Pregame = true;
	bool Loader::Postgame = false;
	std::vector<Component*> Loader::Components;
	Utils::Memory::Allocator Loader::MemAllocator;

	bool Loader::IsPregame()
	{
		return Loader::Pregame;
	}

	void Loader::Initialize()
	{
		Loader::Pregame = true;
		Loader::Postgame = false;
		Loader::MemAllocator.clear();

		Loader::Register(new Flags());
		Loader::Register(new Singleton());

		Loader::Register(new Auth());
		Loader::Register(new Bans());
		Loader::Register(new Bots());
		Loader::Register(new Dvar());
		Loader::Register(new Lean());
		Loader::Register(new Maps());
		Loader::Register(new News());
		Loader::Register(new Node());
		Loader::Register(new RCon());
		Loader::Register(new Menus());
		Loader::Register(new Toast());
		Loader::Register(new Party());
		Loader::Register(new Zones());
		Loader::Register(new Colors());
		Loader::Register(new D3D9Ex());
		Loader::Register(new Logger());
		Loader::Register(new Script());
		Loader::Register(new Weapon());
		Loader::Register(new Window());
		Loader::Register(new Command());
		Loader::Register(new Console());
		Loader::Register(new Friends());
		Loader::Register(new ModList());
		Loader::Register(new Network());
		Loader::Register(new Theatre());
		Loader::Register(new Download());
		Loader::Register(new Playlist());
		Loader::Register(new RawFiles());
		Loader::Register(new Renderer());
		Loader::Register(new UIFeeder());
		Loader::Register(new UIScript());
#ifndef DISABLE_ANTICHEAT
		Loader::Register(new AntiCheat());
#endif
		Loader::Register(new Dedicated());
		Loader::Register(new Discovery());
		Loader::Register(new Exception());
		Loader::Register(new FastFiles());
		Loader::Register(new FrameTime());
		Loader::Register(new Gametypes());
		Loader::Register(new Materials());
		Loader::Register(new Threading());
#ifndef DISABLE_BITMESSAGE
		Loader::Register(new BitMessage());
#endif
		Loader::Register(new FileSystem());
		Loader::Register(new IPCHandler());
		Loader::Register(new ModelSurfs());
		Loader::Register(new PlayerName());
		Loader::Register(new QuickPatch());
		Loader::Register(new ServerInfo());
		Loader::Register(new ServerList());
		Loader::Register(new SlowMotion());
		Loader::Register(new ArenaLength());
		Loader::Register(new StringTable());
		Loader::Register(new ZoneBuilder());
		Loader::Register(new AssetHandler());
		Loader::Register(new Localization());
		Loader::Register(new MusicalTalent());
		Loader::Register(new MinidumpUpload());
		Loader::Register(new StructuredData());
		Loader::Register(new ConnectProtocol());
		Loader::Register(new StartupMessages());

		Loader::Pregame = false;
	}

	void Loader::Uninitialize()
	{
		Loader::PreDestroy();

		std::reverse(Loader::Components.begin(), Loader::Components.end());
		for (auto component : Loader::Components)
		{
#ifdef DEBUG
			if(!Loader::PerformingUnitTests())
			{
				Logger::Print("Unregistering component: %s\n", component->getName());
			}
#endif
			delete component;
		}

		Loader::Components.clear();
		Loader::MemAllocator.clear();
	}

	void Loader::PreDestroy()
	{
		if(!Loader::Postgame)
		{
			Loader::Postgame = true;

			std::reverse(Loader::Components.begin(), Loader::Components.end());
			for (auto component : Loader::Components)
			{
				component->preDestroy();
			}
		}
	}

	bool Loader::PerformUnitTests()
	{
		bool result = true;

		Logger::Print("Performing unit tests for components:\n");

		for (auto component : Loader::Components)
		{
#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
			Logger::Print("Testing '%s'...\n", component->getName());
#endif
			auto startTime = std::chrono::high_resolution_clock::now();
			bool testRes = component->unitTest();
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startTime).count();
			Logger::Print("Test done (%llims): %s\n\n", duration, (testRes ? "Success" : "Error"));
			result &= testRes;
		}

		return result;
	}

	bool Loader::PerformingUnitTests()
	{
#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		return Flags::HasFlag("tests");
#else
		return false;
#endif
	}

	void Loader::Register(Component* component)
	{
		if (component)
		{
#ifdef DEBUG
			if(!Loader::PerformingUnitTests())
			{
				Logger::Print("Component registered: %s\n", component->getName());
			}
#endif
			Loader::Components.push_back(component);
		}
	}

	Utils::Memory::Allocator* Loader::GetAlloctor()
	{
		return &Loader::MemAllocator;
	}
}
