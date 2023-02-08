#include <STDInclude.hpp>
#include "Discord.hpp"
#include "Party.hpp"

#include <discord_rpc.h>

namespace Components
{
	static DiscordRichPresence DiscordPresence;

	bool Discord::Initialized_;

	static void Ready([[maybe_unused]] const DiscordUser* request)
	{
		ZeroMemory(&DiscordPresence, sizeof(DiscordPresence));

		DiscordPresence.instance = 1;

		Logger::Print("Discord: Ready\n");

		Discord_UpdatePresence(&DiscordPresence);
	}

	static void Errored(const int errorCode, const char* message)
	{
		Logger::Print(Game::CON_CHANNEL_ERROR, "Discord: Error (%i): %s\n", errorCode, message);
	}

	void Discord::UpdateDiscord()
	{
		Discord_RunCallbacks();

		if (!Game::CL_IsCgameInitialized())
		{
			DiscordPresence.details = "Multiplayer";
			DiscordPresence.state = "Main Menu";
			DiscordPresence.largeImageKey = "menu_multiplayer";

			DiscordPresence.partySize = 0;
			DiscordPresence.partyMax = 0;
			DiscordPresence.startTimestamp = 0;

			Discord_UpdatePresence(&DiscordPresence);

			return;
		}

		const auto* map = Game::UI_GetMapDisplayName((*Game::ui_mapname)->current.string);

		const Game::StringTable* table;
		Game::StringTable_GetAsset_FastFile("mp/gameTypesTable.csv", &table);
		const auto row = Game::StringTable_LookupRowNumForValue(table, 0, (*Game::ui_gametype)->current.string);
		if (row != -1)
		{
			const auto* value = Game::StringTable_GetColumnValueForRow(table, row, 1);
			const auto* localize = Game::DB_FindXAssetHeader(Game::ASSET_TYPE_LOCALIZE_ENTRY, value).localize;
			DiscordPresence.details = Utils::String::Format("{} on {}", localize->value, map);
		}
		else
		{
			DiscordPresence.details = Utils::String::Format("Team Deathmatch on {}", map);
		}

		const auto* hostName = Game::cls->servername;
		if (std::strcmp(hostName, "localhost") == 0)
		{
			DiscordPresence.state = "Private Match";
		}
		else
		{
			char hostNameBuffer[256]{};
			TextRenderer::StripColors(Party::GetHostName().data(), hostNameBuffer, sizeof(hostNameBuffer));
			TextRenderer::StripAllTextIcons(hostNameBuffer, hostNameBuffer, sizeof(hostNameBuffer));

			DiscordPresence.state = hostNameBuffer;
		}

		DiscordPresence.partySize = 0;
		DiscordPresence.partyMax = Party::GetMaxClients();

		if (!DiscordPresence.startTimestamp)
		{
			DiscordPresence.startTimestamp = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		}

		DiscordPresence.largeImageKey = "menu_multiplayer";

		Discord_UpdatePresence(&DiscordPresence);
	}

	Discord::Discord()
	{
		if (Dedicated::IsEnabled() || ZoneBuilder::IsEnabled())
		{
			return;
		}

		DiscordEventHandlers handlers;
		ZeroMemory(&handlers, sizeof(handlers));
		handlers.ready = Ready;
		handlers.errored = Errored;
		handlers.disconnected = Errored;
		handlers.joinGame = nullptr;
		handlers.spectateGame = nullptr;
		handlers.joinRequest = nullptr;

		Discord_Initialize("1072930169385394288", &handlers, 1, nullptr);

		Scheduler::Once(UpdateDiscord, Scheduler::Pipeline::MAIN);
		Scheduler::Loop(UpdateDiscord, Scheduler::Pipeline::MAIN, 15s);

		Initialized_ = true;
	}

	void Discord::preDestroy()
	{
		if (Dedicated::IsEnabled() || ZoneBuilder::IsEnabled() || !Initialized_)
		{
			return;
		}

		Discord_Shutdown();
	}
}
