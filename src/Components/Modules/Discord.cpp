#include <STDInclude.hpp>
#include "Discord.hpp"
#include "Party.hpp"
#include "TextRenderer.hpp"

#include <discord_rpc.h>

namespace Components
{
	static DiscordRichPresence DiscordPresence;

	bool Discord::Initialized_;

	static unsigned int GetDiscordNonce()
	{
		static auto nonce = Utils::Cryptography::Rand::GenerateInt();
		return nonce;
	}

	static void Ready([[maybe_unused]] const DiscordUser* request)
	{
		ZeroMemory(&DiscordPresence, sizeof(DiscordPresence));

		DiscordPresence.instance = 1;

		Logger::Print("Discord: Ready\n");

		Discord_UpdatePresence(&DiscordPresence);
	}

	static void JoinGame(const char* joinSecret)
	{
		Game::Cbuf_AddText(0, Utils::String::VA("connect %s\n", joinSecret));
	}

	static void JoinRequest(const DiscordUser* request)
	{
		Logger::Debug("Discord: Join request from {} ({})\n", request->username, request->userId);
		Discord_Respond(request->userId, DISCORD_REPLY_IGNORE);
	}

	static void Errored(const int errorCode, const char* message)
	{
		Logger::Print(Game::CON_CHANNEL_ERROR, "Discord: Error ({}): {}\n", errorCode, message);
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

		char hostNameBuffer[256]{};

		const auto* map = Game::UI_GetMapDisplayName((*Game::ui_mapname)->current.string);

		const Game::StringTable* table;
		Game::StringTable_GetAsset_FastFile("mp/gameTypesTable.csv", &table);
		const auto row = Game::StringTable_LookupRowNumForValue(table, 0, (*Game::ui_gametype)->current.string);
		if (row != -1)
		{
			const auto* value = Game::StringTable_GetColumnValueForRow(table, row, 1);
			const auto* localize = Game::DB_FindXAssetHeader(Game::ASSET_TYPE_LOCALIZE_ENTRY, value).localize;
			DiscordPresence.details = Utils::String::Format("{} on {}", localize ? localize->value : "Team Deathmatch", map);
		}
		else
		{
			DiscordPresence.details = Utils::String::Format("Team Deathmatch on {}", map);
		}

		const auto* hostName = Game::cls->servername;
		if (std::strcmp(hostName, "localhost") == 0)
		{
			DiscordPresence.state = "Private Match";
			DiscordPresence.partyPrivacy = DISCORD_PARTY_PRIVATE;
		}
		else
		{
			TextRenderer::StripColors(Party::GetHostName().data(), hostNameBuffer, sizeof(hostNameBuffer));
			TextRenderer::StripAllTextIcons(hostNameBuffer, hostNameBuffer, sizeof(hostNameBuffer));

			DiscordPresence.state = hostNameBuffer;
			DiscordPresence.partyPrivacy = DISCORD_PARTY_PUBLIC;
		}

		std::hash<Network::Address> hashFn;
		const auto address = Party::Target();

		DiscordPresence.partyId = Utils::String::VA("%s - %zu", hostNameBuffer, hashFn(address) ^ GetDiscordNonce());
		DiscordPresence.joinSecret = address.getCString();
		DiscordPresence.partySize = Game::cgArray[0].snap ? Game::cgArray[0].snap->numClients : 1;
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
		handlers.joinGame = JoinGame;
		handlers.spectateGame = nullptr;
		handlers.joinRequest = JoinRequest;

		Discord_Initialize("1072930169385394288", &handlers, 1, nullptr);

		Scheduler::Once(UpdateDiscord, Scheduler::Pipeline::MAIN);
		Scheduler::Loop(UpdateDiscord, Scheduler::Pipeline::MAIN, 15s);

		Initialized_ = true;
	}

	void Discord::preDestroy()
	{
		if (!Initialized_)
		{
			return;
		}

		Discord_Shutdown();
	}
}
