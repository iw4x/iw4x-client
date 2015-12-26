namespace Components
{
	class Component
	{
	public:
		Component() {};
		virtual ~Component() {};
		virtual const char* GetName() { return "Unknown"; };
	};

	class Loader
	{
	public:
		static void Initialize();
		static void Uninitialize();
		static void Register(Component* component);

	private:
		static std::vector<Component*> Components;
	};
}

#include "Dvar.hpp"
#include "Maps.hpp"
#include "Menus.hpp"
#include "Colors.hpp"
#include "Logger.hpp"
#include "Window.hpp"
#include "Command.hpp"
#include "Console.hpp"
#include "Network.hpp"
#include "Party.hpp" // Destroys the order, but requires network classes :D
#include "RawFiles.hpp"
#include "Renderer.hpp"
#include "FastFiles.hpp"
#include "Materials.hpp"
#include "FileSystem.hpp"
#include "QuickPatch.hpp"
#include "AssetHandler.hpp"
#include "Localization.hpp"
#include "MusicalTalent.hpp"
