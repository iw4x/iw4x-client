#include <Utils/InfoString.hpp>

#include "Loader.hpp"

#include "Modules/ArenaLength.hpp"
#include "Modules/Auth.hpp"
#include "Modules/Bans.hpp"
#include "Modules/Bots.hpp"
#include "Modules/Branding.hpp"
#include "Modules/Bullet.hpp"
#include "Modules/CardTitles.hpp"
#include "Modules/Ceg.hpp"
#include "Modules/Changelog.hpp"
#include "Modules/Chat.hpp"
#include "Modules/ClanTags.hpp"
#include "Modules/ClientCommand.hpp"
#include "Modules/ConnectProtocol.hpp"
#include "Modules/Console.hpp"
#include "Modules/ConfigStrings.hpp"
#include "Modules/D3D9Ex.hpp"
#include "Modules/Debug.hpp"
#include "Modules/Discord.hpp"
#include "Modules/Discovery.hpp"
#include "Modules/Download.hpp"
#include "Modules/Elevators.hpp"
#include "Modules/Events.hpp"
#include "Modules/Exception.hpp"
#include "Modules/FastFiles.hpp"
#include "Modules/Friends.hpp"
#include "Modules/Gamepad.hpp"
#include "Modules/Huffman.hpp"
#include "Modules/IPCPipe.hpp"
#include "Modules/Lean.hpp"
#include "Modules/MapDump.hpp"
#include "Modules/MapRotation.hpp"
#include "Modules/Materials.hpp"
#include "Modules/ModList.hpp"
#include "Modules/ModelCache.hpp"
#include "Modules/ModelSurfs.hpp"
#include "Modules/NetworkDebug.hpp"
#include "Modules/News.hpp"
#include "Modules/Node.hpp"
#include "Modules/Party.hpp"
#include "Modules/PlayerMovement.hpp"
#include "Modules/PlayerName.hpp"
#include "Modules/Playlist.hpp"
#include "Modules/QuickPatch.hpp"
#include "Modules/RawFiles.hpp"
#include "Modules/RawMouse.hpp"
#include "Modules/RCon.hpp"
#include "Modules/Rumble.hpp"
#include "Modules/Security.hpp"
#include "Modules/ServerCommands.hpp"
#include "Modules/ServerInfo.hpp"
#include "Modules/ServerList.hpp"
#include "Modules/Session.hpp"
#include "Modules/SlowMotion.hpp"
#include "Modules/StartupMessages.hpp"
#include "Modules/Stats.hpp"
#include "Modules/StringTable.hpp"
#include "Modules/StructuredData.hpp"
#include "Modules/TextRenderer.hpp"
#include "Modules/Theatre.hpp"
#include "Modules/Threading.hpp"
#include "Modules/Toast.hpp"
#include "Modules/UIFeeder.hpp"
#include "Modules/Updater.hpp"
#include "Modules/VisionFile.hpp"
#include "Modules/Voice.hpp"
#include "Modules/Vote.hpp"
#include "Modules/Weapon.hpp"
#include "Modules/Window.hpp"
#include "Modules/BotLib/lPrecomp.hpp"

namespace Components
{
	void Loader::Initialize()
	{
		Utils::Memory::GetAllocator()->clear();

		static Singleton s{};
		static Auth Auth_{};
		static Command Command_{};
		static Dvar Dvar_{};
		static Exception Exception_{};
		static IPCPipe IPCPipe_{};
		static Network Network_{};
		static Logger Logger_{};
		static UIScript UIScript_{};
		static ZoneBuilder ZoneBuilder_{};
		static ConfigStrings ConfigStrings_{};
		static ArenaLength ArenaLength_{};
		static AssetHandler AssetHandler_{};
		static Bans Bans_{};
		static Bots Bots_{};
		static Branding Branding_{};
		static Bullet Bullet_{};
		static CardTitles CardTitles_{};
		static Ceg Ceg_{};
		static Changelog Changelog_{};
		static Chat Chat_{};
		static ClanTags ClanTags_{};
		static ClientCommand ClientCommand_{};
		static ConnectProtocol ConnectProtocol_{};
		static Console Console_{};
		static D3D9Ex D3D9Ex_{};
		static Debug Debug_{};
		static Dedicated Dedicated_{};
		static Discord Discord_{};
		static Discovery Discovery_{};
		static Download Download_{};
		static Elevators Elevators_{};
		static Events Events_{};
		static FastFiles FastFiles_{};
		static FileSystem FileSystem_{};
		static Friends Friends_{};
		static Gamepad Gamepad_{};
		static Rumble Rumble_{};
		static Huffman Huffman_{};
		static Lean Lean_{};
		static Localization Localization_{};
		static MapDump MapDump_{};
		static MapRotation MapRotation_{};
		static Maps Maps_{};
		static Materials Materials_{};
		static Menus Menus_{};
		static ModList ModList_{};
		static ModelCache ModelCache_{};
		static ModelSurfs ModelSurfs_{};
		static NetworkDebug NetworkDebug_{};
		static News News_{};
		static Node Node_{};
		static Party Party_{};
		static PlayerMovement PlayerMovement_{};
		static PlayerName PlayerName_{};
		static Playlist Playlist_{};
		static QuickPatch QuickPatch_{};
		static RawFiles RawFiles_{};
		static RawMouse RawMouse_{};
		static RCon RCon_{};
		static Renderer Renderer_{};
		static Scheduler Scheduler_{};
		static Security Security_{};
		static ServerCommands ServerCommands_{};
		static ServerInfo ServerInfo_{};
		static ServerList ServerList_{};
		static Session Session_{};
		static SlowMotion SlowMotion_{};
		static StartupMessages StartupMessages_{};
		static Stats Stats_{};
		static StringTable StringTable_{};
		static StructuredData StructuredData_{};
		static TextRenderer TextRenderer_{};
		static Theatre Theatre_{};
		static Threading Threading_{};
		static Toast Toast_{};
		static UIFeeder UIFeeder_{};
		static Updater Updater_{};
		static VisionFile VisionFile_{};
		static Voice Voice_{};
		static Vote Vote_{};
		static Weapon Weapon_{};
		static Window Window_{};
		static Zones Zones_{};
		static GSC::GSC GSC_{};
		static BotLib::lPrecomp BotLib_{};
	}
}
