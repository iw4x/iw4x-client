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

#include "Modules\Dvar.hpp"
#include "Modules\Maps.hpp"
#include "Modules\Menus.hpp"
#include "Modules\Colors.hpp"
#include "Modules\Logger.hpp"
#include "Modules\Window.hpp"
#include "Modules\Command.hpp"
#include "Modules\Console.hpp"
#include "Modules\Network.hpp"
#include "Modules\Party.hpp" // Destroys the order, but requires network classes :D
#include "Modules\RawFiles.hpp"
#include "Modules\Renderer.hpp"
#include "Modules\UIFeeder.hpp"
#include "Modules\UIScript.hpp"
#include "Modules\Dedicated.hpp"
#include "Modules\FastFiles.hpp"
#include "Modules\Materials.hpp"
#include "Modules\Singleton.hpp"
#include "Modules\FileSystem.hpp"
#include "Modules\QuickPatch.hpp"
#include "Modules\ServerList.hpp"
#include "Modules\AssetHandler.hpp"
#include "Modules\Localization.hpp"
#include "Modules\MusicalTalent.hpp"
