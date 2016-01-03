#include "..\STDInclude.hpp"

namespace Components
{
	std::vector<Component*> Loader::Components;

	void Loader::Initialize()
	{
		Loader::Register(new Dedicated());
		Loader::Register(new Singleton());

		Loader::Register(new Dvar());
		Loader::Register(new Maps());
		Loader::Register(new News());
		Loader::Register(new Menus());
		Loader::Register(new Party());
		Loader::Register(new Colors());
		Loader::Register(new Logger());
		Loader::Register(new Window());
		Loader::Register(new Command());
		Loader::Register(new Console());
		Loader::Register(new IPCPipe());
		Loader::Register(new Network());
		Loader::Register(new Playlist());
		Loader::Register(new RawFiles());
		Loader::Register(new Renderer());
		Loader::Register(new UIFeeder());
		Loader::Register(new UIScript());
		Loader::Register(new FastFiles());
		Loader::Register(new Discovery());
		Loader::Register(new Materials());
		Loader::Register(new FileSystem());
		Loader::Register(new QuickPatch());
		Loader::Register(new ServerList());
		Loader::Register(new AssetHandler());
		Loader::Register(new Localization());
		Loader::Register(new MusicalTalent());
		Loader::Register(new ConnectProtocol());
	}

	void Loader::Uninitialize()
	{
		for (auto component : Loader::Components)
		{
			Logger::Print("Unregistering component: %s", component->GetName());
			delete component;
		}

		Loader::Components.clear();
	}

	void Loader::Register(Component* component)
	{
		if (component)
		{
			Logger::Print("Component registered: %s", component->GetName());
			Loader::Components.push_back(component);
		}
	}
}
