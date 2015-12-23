#include "..\STDInclude.hpp"

namespace Components
{
	std::vector<Component*> Loader::Components;

	void Loader::Initialize()
	{
		Loader::Register(new Dvar());
		Loader::Register(new Colors());
		Loader::Register(new Window());
		Loader::Register(new Command());
		Loader::Register(new Console());
		Loader::Register(new RawFiles());
		Loader::Register(new Renderer());
		Loader::Register(new Materials());
		Loader::Register(new QuickPatch());
	}

	void Loader::Uninitialize()
	{
		for (auto component : Loader::Components)
		{
			OutputDebugStringA(Utils::VA("Unregistering component: %s", component->GetName()));
			delete component;
		}

		Loader::Components.clear();
	}

	void Loader::Register(Component* component)
	{
		if (component)
		{
			OutputDebugStringA(Utils::VA("Component registered: %s", component->GetName()));
			Loader::Components.push_back(component);
		}
	}
}
