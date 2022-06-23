#include <STDInclude.hpp>

namespace Components
{
	std::string Playlist::CurrentPlaylistBuffer;
	std::string Playlist::ReceivedPlaylistBuffer;
	std::unordered_map<const void*, std::string> Playlist::MapRelocation;

	void Playlist::LoadPlaylist()
	{
		// Check if playlist already loaded
		if (Utils::Hook::Get<bool>(0x1AD3680)) return;

		// Don't load playlists when dedi and no party
		if (Dedicated::IsEnabled() && !Dvar::Var("party_enable").get<bool>())
		{
			Utils::Hook::Set<bool>(0x1AD3680, true); // Set received to true
			Dvar::Var("xblive_privateserver").set(true);
			return;
		}

		Dvar::Var("xblive_privateserver").set(false);

		auto playlistFilename = Dvar::Var("playlistFilename").get<std::string>();
		FileSystem::File playlist(playlistFilename);

		if (playlist.exists())
		{
			Logger::Print("Parsing playlist '{}'...\n", playlist.getName());
			Game::Playlist_ParsePlaylists(playlist.getBuffer().data());
			Utils::Hook::Set<bool>(0x1AD3680, true); // Playlist loaded
		}
		else
		{
			Logger::Print("Unable to load playlist '{}'!\n", playlist.getName());
		}
	}

	DWORD Playlist::StorePlaylistStub(const char** buffer)
	{
		Playlist::MapRelocation.clear();
		Playlist::CurrentPlaylistBuffer = Utils::Compression::ZLib::Compress(*buffer);
		return Utils::Hook::Call<DWORD(const char**)>(0x4C0350)(buffer);
	}

	void Playlist::PlaylistRequest(const Network::Address& address, [[maybe_unused]] const std::string& data)
	{
		const auto password = Dvar::Var("g_password").get<std::string>();

		if (password.length())
		{
			if (password != data)
			{
				Network::SendCommand(address, "playlistInvalidPassword");
				return;
			}
		}

		Logger::Print("Received playlist request, sending currently stored buffer.\n");

		std::string compressedList = Playlist::CurrentPlaylistBuffer;

		Proto::Party::Playlist list;
		list.set_hash(Utils::Cryptography::JenkinsOneAtATime::Compute(compressedList));
		list.set_buffer(compressedList);

		Network::SendCommand(address, "playlistResponse", list.SerializeAsString());
	}

	void Playlist::PlaylistReponse(const Network::Address& address, [[maybe_unused]] const std::string& data)
	{
		if (Party::PlaylistAwaiting())
		{
			if (address == Party::Target())
			{
				Proto::Party::Playlist list;

				if (!list.ParseFromString(data))
				{
					Party::PlaylistError(Utils::String::VA("Received playlist response from %s, but it is invalid.", address.getCString()));
					Playlist::ReceivedPlaylistBuffer.clear();
					return;
				}
				else
				{
					// Generate buffer and hash
					const auto& compressedData = list.buffer();
					const auto hash = Utils::Cryptography::JenkinsOneAtATime::Compute(compressedData);

					//Validate hashes
					if (hash != list.hash())
					{
						Party::PlaylistError(Utils::String::VA("Received playlist response from %s, but the checksum did not match (%X != %X).", address.getCString(), list.hash(), hash));
						Playlist::ReceivedPlaylistBuffer.clear();
						return;
					}

					// Decompress buffer
					Playlist::ReceivedPlaylistBuffer = Utils::Compression::ZLib::Decompress(compressedData);

					// Load and continue connection
					Logger::Print("Received playlist, loading and continuing connection...\n");
					Game::Playlist_ParsePlaylists(Playlist::ReceivedPlaylistBuffer.data());
					Party::PlaylistContinue();
				}
			}
			else
			{
				Logger::Print("Received playlist from someone else than our target host, ignoring it.\n");
			}
		}
		else
		{
			Logger::Print("Received stray playlist response, ignoring it.\n");
		}
	}

	void Playlist::PlaylistInvalidPassword([[maybe_unused]] const Network::Address& address, [[maybe_unused]] const std::string& data)
	{
		Party::PlaylistError("Error: Invalid Password for Party.");
	}

	void Playlist::MapNameCopy(char *dest, const char *src, int destsize)
	{
		Utils::Hook::Call<void(char*, const char*, int)>(0x4D6F80)(dest, src, destsize);
		Playlist::MapRelocation[dest] = src;
	}

	void Playlist::SetMapName(const char* cvar, const char* value)
	{
		auto i = Playlist::MapRelocation.find(value);
		if (i != Playlist::MapRelocation.end())
		{
			value = i->second.data();
		}

		Game::Dvar_SetStringByName(cvar, value);
	}

	int Playlist::GetMapIndex(const char* mapname)
	{
		auto i = Playlist::MapRelocation.find(mapname);
		if (i != Playlist::MapRelocation.end())
		{
			mapname = i->second.data();
		}

		return Game::Live_GetMapIndex(mapname);
	}

	Playlist::Playlist()
	{
		// Default playlists
		Utils::Hook::Set<const char*>(0x60B06E, "playlists_default.info");

		// disable playlist download function
		Utils::Hook::Set<BYTE>(0x4D4790, 0xC3);

		// Load playlist, but don't delete it
		Utils::Hook::Nop(0x4D6EBB, 5);
		Utils::Hook::Nop(0x4D6E67, 5);
		Utils::Hook::Nop(0x4D6E71, 2);

		// playlist dvar 'validity check'
		Utils::Hook::Set<BYTE>(0x4B1170, 0xC3);

		// disable playlist checking
		Utils::Hook::Set<BYTE>(0x5B69E9, 0xEB); // too new
		Utils::Hook::Set<BYTE>(0x5B696E, 0xEB); // too old

		//Got playlists is true
		//Utils::Hook::Set<bool>(0x1AD3680, true);

		Utils::Hook(0x497DB5, Playlist::GetMapIndex, HOOK_CALL).install()->quick();
		Utils::Hook(0x42A19D, Playlist::MapNameCopy, HOOK_CALL).install()->quick();
		Utils::Hook(0x4A6FEE, Playlist::SetMapName, HOOK_CALL).install()->quick();

		// Store playlist buffer on load
		Utils::Hook(0x42961C, Playlist::StorePlaylistStub, HOOK_CALL).install()->quick();

		//if (Dedicated::IsDedicated())
		{
			// Custom playlist loading
			Utils::Hook(0x420B5A, Playlist::LoadPlaylist, HOOK_JUMP).install()->quick();

			// disable playlist.ff loading function
			Utils::Hook::Set<BYTE>(0x4D6E60, 0xC3);
		}

		Network::OnPacket("getPlaylist", PlaylistRequest);
		Network::OnPacket("playlistResponse", PlaylistReponse);
		Network::OnPacket("playlistInvalidPassword", PlaylistInvalidPassword);
	}
}
