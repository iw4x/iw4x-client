#include <STDInclude.hpp>
#include <Utils/Compression.hpp>

#include <proto/party.pb.h>

#include "Party.hpp"
#include "Playlist.hpp"

namespace Components
{
	std::string Playlist::CurrentPlaylistBuffer;
	std::string Playlist::ReceivedPlaylistBuffer;
	std::unordered_map<const void*, std::string> Playlist::MapRelocation;

	void Playlist::LoadPlaylist()
	{
		// Check if playlist already loaded
		if (*Game::s_havePlaylists) return;

		// Don't load playlists when dedi and no party
		if (Dedicated::IsEnabled() && !Party::IsEnabled())
		{
			*Game::s_havePlaylists = true;
			Dvar::Var("xblive_privateserver").set(true);
			return;
		}

		Dvar::Var("xblive_privateserver").set(false);

		const auto playlistFilename = Dvar::Var("playlistFilename").get<std::string>();
		FileSystem::File playlist(playlistFilename);

		if (playlist.exists())
		{
			Logger::Print("Parsing playlist '{}'...\n", playlist.getName());
			Game::Playlist_ParsePlaylists(playlist.getBuffer().data());
			*Game::s_havePlaylists = true;
		}
		else
		{
			Logger::Print("Unable to load playlist '{}'!\n", playlist.getName());
		}
	}

	char* Playlist::Com_ParseOnLine_Hk(const char** data_p)
	{
		MapRelocation.clear();
		CurrentPlaylistBuffer = Utils::Compression::ZLib::Compress(*data_p);
		return Game::Com_ParseOnLine(data_p);
	}

	void Playlist::PlaylistRequest(const Network::Address& address, [[maybe_unused]] const std::string& data)
	{
		const auto* password = *Game::g_password ? (*Game::g_password)->current.string : "";

		if (*password)
		{
			if (password != data)
			{
				Network::SendCommand(address, "playlistInvalidPassword");
				return;
			}
		}

		Logger::Print("Received playlist request, sending currently stored buffer.\n");

		std::string compressedList = CurrentPlaylistBuffer;

		Proto::Party::Playlist list;
		list.set_hash(Utils::Cryptography::JenkinsOneAtATime::Compute(compressedList));
		list.set_buffer(compressedList);

		Network::SendCommand(address, "playlistResponse", list.SerializeAsString());
	}

	void Playlist::PlaylistResponse(const Network::Address& address, [[maybe_unused]] const std::string& data)
	{
		if (!Party::PlaylistAwaiting())
		{
			Logger::Print("Received stray playlist response, ignoring it.\n");
			return;
		}
			
		if (address != Party::Target())
		{
			Logger::Print("Received playlist from someone else than our target host, ignoring it.\n");
			return;
		}
		
		Proto::Party::Playlist list;

		if (!list.ParseFromString(data))
		{
			Party::PlaylistError(std::format("Received playlist response from {}, but it is invalid.", address.getString()));
			ReceivedPlaylistBuffer.clear();
		}
		else
		{
			// Generate buffer and hash
			const auto& compressedData = list.buffer();
			const auto hash = Utils::Cryptography::JenkinsOneAtATime::Compute(compressedData);

			// Validate hashes
			if (hash != list.hash())
			{
				Party::PlaylistError(std::format("Received playlist response from {}, but the checksum did not match ({} != {}).", address.getString(), list.hash(), hash));
				ReceivedPlaylistBuffer.clear();
				return;
			}

			// Decompress buffer
			ReceivedPlaylistBuffer = Utils::Compression::ZLib::Decompress(compressedData);

			// Load and continue connection
			Logger::Print("Received playlist, loading and continuing connection...\n");
			Game::Playlist_ParsePlaylists(ReceivedPlaylistBuffer.data());
			Party::PlaylistContinue();
		}
	}

	void Playlist::PlaylistInvalidPassword([[maybe_unused]] const Network::Address& address, [[maybe_unused]] const std::string& data)
	{
		Party::PlaylistError("Error: Invalid Password for Party.");
	}

	void Playlist::MapNameCopy(char* dest, const char* src, int destsize)
	{
		Utils::Hook::Call<void(char*, const char*, int)>(0x4D6F80)(dest, src, destsize);
		MapRelocation[dest] = src;
	}

	void Playlist::SetMapName(const char* dvarName, const char* value)
	{
		auto i = MapRelocation.find(value);
		if (i != MapRelocation.end())
		{
			value = i->second.data();
		}

		Game::Dvar_SetStringByName(dvarName, value);
	}

	int Playlist::GetMapIndex(const char* mapname)
	{
		auto i = MapRelocation.find(mapname);
		if (i != MapRelocation.end())
		{
			mapname = i->second.data();
		}

		return Game::Live_GetMapIndex(mapname);
	}

	Playlist::Playlist()
	{
		// Default playlists
		Utils::Hook::Set<const char*>(0x60B06E, "playlists_default.info");

		// Disable playlist download function
		Utils::Hook::Set<BYTE>(0x4D4790, 0xC3);

		// Load playlist, but don't delete it
		Utils::Hook::Nop(0x4D6EBB, 5);
		Utils::Hook::Nop(0x4D6E67, 5);
		Utils::Hook::Nop(0x4D6E71, 2);

		// Disable Playlist_ValidatePlaylistNum
		Utils::Hook::Set<BYTE>(0x4B1170, 0xC3);

		// Disable playlist checking
		Utils::Hook::Set<BYTE>(0x5B69E9, 0xEB); // Too new
		Utils::Hook::Set<BYTE>(0x5B696E, 0xEB); // Too old

		// Got playlists is true
		//Utils::Hook::Set<bool>(0x1AD3680, true);

		Utils::Hook(0x497DB5, GetMapIndex, HOOK_CALL).install()->quick();
		Utils::Hook(0x42A19D, MapNameCopy, HOOK_CALL).install()->quick();
		Utils::Hook(0x4A6FEE, SetMapName, HOOK_CALL).install()->quick();

		// Store playlist buffer on load
		Utils::Hook(0x42961C, Com_ParseOnLine_Hk, HOOK_CALL).install()->quick(); // Playlist_ParsePlaylists

		//if (Dedicated::IsDedicated())
		{
			// Custom playlist loading
			Utils::Hook(0x420B5A, LoadPlaylist, HOOK_JUMP).install()->quick();

			// disable playlist.ff loading function (Win_LoadPlaylistFastfile)
			Utils::Hook::Set<std::uint8_t>(0x4D6E60, 0xC3);
		}

		Network::OnClientPacket("getPlaylist", PlaylistRequest);
		Network::OnClientPacket("playlistResponse", PlaylistResponse);
		Network::OnClientPacket("playlistInvalidPassword", PlaylistInvalidPassword);
	}
}
