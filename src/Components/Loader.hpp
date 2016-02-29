namespace Components
{
	class Component
	{
	public:
		Component() {};
		virtual ~Component() {};
		virtual const char* GetName() { return "Unknown"; };
		virtual bool UnitTest() { return true; }; // Unit testing entry
	};

	class Loader
	{
	public:
		static void Initialize();
		static void Uninitialize();
		static bool PerformUnitTests();
		static bool PerformingUnitTests();
		static void Register(Component* component);

	private:
		static std::vector<Component*> Components;
	};
}

#include "Modules\Auth.hpp"
#include "Modules\Dvar.hpp"
#include "Modules\Maps.hpp"
#include "Modules\News.hpp"
#include "Modules\Flags.hpp"
#include "Modules\Menus.hpp"
#include "Modules\Colors.hpp"
#include "Modules\D3D9Ex.hpp"
#include "Modules\Logger.hpp"
#include "Modules\Weapon.hpp"
#include "Modules\Window.hpp"
#include "Modules\Command.hpp"
#include "Modules\Console.hpp"
#include "Modules\IPCPipe.hpp"
#include "Modules\Network.hpp"
#include "Modules\Theatre.hpp"
#include "Modules\Node.hpp"
#include "Modules\RCon.hpp"
#include "Modules\Party.hpp" // Destroys the order, but requires network classes :D
#include "Modules\Download.hpp"
#include "Modules\Playlist.hpp"
#include "Modules\RawFiles.hpp"
#include "Modules\Renderer.hpp"
#include "Modules\UIFeeder.hpp"
#include "Modules\UIScript.hpp"
#include "Modules\AntiCheat.hpp"
#include "Modules\Dedicated.hpp"
#include "Modules\Discovery.hpp"
#include "Modules\Exception.hpp"
#include "Modules\FastFiles.hpp"
#include "Modules\Materials.hpp"
#include "Modules\Singleton.hpp"
#include "Modules\FileSystem.hpp"
#include "Modules\QuickPatch.hpp"
#include "Modules\ServerInfo.hpp"
#include "Modules\ServerList.hpp"
#include "Modules\StringTable.hpp"
#include "Modules\ZoneBuilder.hpp"
#include "Modules\AssetHandler.hpp"
#include "Modules\Localization.hpp"
#include "Modules\MusicalTalent.hpp"
#include "Modules\StructuredData.hpp"
#include "Modules\ConnectProtocol.hpp"
