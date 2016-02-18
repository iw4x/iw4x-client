#include "STDInclude.hpp"

namespace Components
{
	std::vector<Component*> Loader::Components;

	void Loader::Initialize()
	{
		Loader::Register(new Flags());
		Loader::Register(new Singleton());

		Loader::Register(new Dvar());
		Loader::Register(new Maps());
		Loader::Register(new News());
		Loader::Register(new Node());
		Loader::Register(new RCon());
		Loader::Register(new Menus());
		Loader::Register(new Party());
		Loader::Register(new Colors());
		Loader::Register(new D3D9Ex());
		Loader::Register(new Logger());
		Loader::Register(new Weapon());
		Loader::Register(new Window());
		Loader::Register(new Command());
		Loader::Register(new Console());
		Loader::Register(new IPCPipe());
		Loader::Register(new Network());
		Loader::Register(new Theatre());
		Loader::Register(new Download());
		Loader::Register(new Playlist());
		Loader::Register(new RawFiles());
		Loader::Register(new Renderer());
		Loader::Register(new UIFeeder());
		Loader::Register(new UIScript());
		Loader::Register(new Dedicated());
		Loader::Register(new Discovery());
		Loader::Register(new Exception());
		Loader::Register(new FastFiles());
		Loader::Register(new Materials());
		Loader::Register(new FileSystem());
		Loader::Register(new QuickPatch());
		Loader::Register(new ServerInfo());
		Loader::Register(new ServerList());
		Loader::Register(new StringTable());
		Loader::Register(new ZoneBuilder());
		Loader::Register(new AssetHandler());
		Loader::Register(new Localization());
		Loader::Register(new MusicalTalent());
		Loader::Register(new StructuredData());
		Loader::Register(new ConnectProtocol());
	}

	void Loader::Uninitialize()
	{
		std::reverse(Loader::Components.begin(), Loader::Components.end());
		for (auto component : Loader::Components)
		{
			Logger::Print("Unregistering component: %s\n", component->GetName());
			delete component;
		}

		Loader::Components.clear();
	}

	bool Loader::PerformUnitTests()
	{
		bool result = true;

		Logger::Print("Performing unit tests for components:\n");

		for (auto component : Loader::Components)
		{
			Logger::Print("Testing '%s'...\n", component->GetName());
			auto startTime = std::chrono::high_resolution_clock::now();
			bool testRes = component->UnitTest();
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - startTime).count();
			Logger::Print("Test done (%llims): %s\n\n", duration, (testRes ? "Success" : "Error"));
			result &= testRes;
		}

		return result;
	}

	bool Loader::PerformingUnitTests()
	{
#if DEBUG
		return Flags::HasFlag("tests");
#else
		return false;
#endif
	}

	void Loader::Register(Component* component)
	{
		if (component)
		{
			Logger::Print("Component registered: %s\n", component->GetName());
			Loader::Components.push_back(component);
		}
	}
}
