#include "STDInclude.hpp"

namespace Components
{
	std::string Playlist::CurrentPlaylistBuffer;
	std::string Playlist::ReceivedPlaylistBuffer;

	void Playlist::LoadPlaylist()
	{
		// Check if playlist already loaded
		if (Utils::Hook::Get<bool>(0x1AD3680)) return;

		// Don't load playlists when dedi and no party
		if (Dedicated::IsDedicated() && !Dvar::Var("party_enable").Get<bool>())
		{
			Utils::Hook::Set<bool>(0x1AD3680, true); // Set received to true
			Dvar::Var("xblive_privateserver").Set(true);
			return;
		}

		Dvar::Var("xblive_privateserver").Set(false);

		std::string playlistFilename = Dvar::Var("playlistFilename").Get<char*>();
		FileSystem::File playlist(playlistFilename);

		if (playlist.Exists())
		{
			Logger::Print("Parsing playlist '%s'...\n", playlist.GetName().data());
			Game::Live_ParsePlaylists(playlist.GetBuffer().data());
			Utils::Hook::Set<bool>(0x1AD3680, true); // Playlist loaded
		}
		else
		{
			Logger::Print("Unable to load playlist '%s'!\n", playlist.GetName().data());
		}
	}

	DWORD Playlist::StorePlaylistStub(const char** buffer)
	{
		Playlist::CurrentPlaylistBuffer = *buffer;
		return Utils::Hook::Call<DWORD(const char**)>(0x4C0350)(buffer);
	}

	void Playlist::PlaylistRequest(Network::Address address, std::string data)
	{
		Logger::Print("Received playlist request, sending currently stored buffer.\n");

		std::string compressedList = Utils::Compression::ZLib::Compress(Playlist::CurrentPlaylistBuffer);
		unsigned int size = compressedList.size();
		unsigned int hash = Utils::OneAtATime(compressedList.data(), compressedList.size());

		std::string response = "playlistresponse\n";
		response.append(reinterpret_cast<char*>(&hash), 4);
		response.append(reinterpret_cast<char*>(&size), 4);
		response.append(compressedList);

		Network::SendRaw(address, response);
	}

	void Playlist::PlaylistReponse(Network::Address address, std::string data)
	{
		if (Party::PlaylistAwaiting())
		{
			if (address == Party::Target())
			{
				if (data.size() <= 8)
				{
					Party::PlaylistError(Utils::VA("Received playlist response, but it is invalid."));
					Playlist::ReceivedPlaylistBuffer.clear();
					return;
				}
				else
				{
					// Read hash and length
					unsigned int hash = *reinterpret_cast<unsigned int*>(const_cast<char*>(data.data()));
					unsigned int length = *reinterpret_cast<unsigned int*>(const_cast<char*>(data.data() + 4));

					// Verify length
					if (length > (data.size() - 8))
					{
						Party::PlaylistError(Utils::VA("Received playlist response, but it is too short."));
						Playlist::ReceivedPlaylistBuffer.clear();
						return;
					}

					// Generate buffer and hash
					std::string compressedData(data.data() + 8, length);
					unsigned int hash2 = Utils::OneAtATime(compressedData.data(), compressedData.size());

					//Validate hashes
					if (hash2 != hash)
					{
						Party::PlaylistError(Utils::VA("Received playlist response, but the checksum did not match (%d != %d).", hash, hash2));
						Playlist::ReceivedPlaylistBuffer.clear();
						return;
					}

					// Decompress buffer
					Playlist::ReceivedPlaylistBuffer = Utils::Compression::ZLib::Decompress(compressedData);

					// Load and continue connection
					Logger::Print("Received playlist, loading and continuing connection...\n");
					Game::Live_ParsePlaylists(Playlist::ReceivedPlaylistBuffer.data());
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

	Playlist::Playlist()
	{
		// Default playlists
		Utils::Hook::Set<char*>(0x60B06E, "playlists_default.info");

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

		// Store playlist buffer on load
		Utils::Hook(0x42961C, Playlist::StorePlaylistStub, HOOK_CALL).Install()->Quick();

		//if (Dedicated::IsDedicated())
		{
			// Custom playlist loading
			Utils::Hook(0x420B5A, Playlist::LoadPlaylist, HOOK_JUMP).Install()->Quick();

			// disable playlist.ff loading function
			Utils::Hook::Set<BYTE>(0x4D6E60, 0xC3);
		}

		Network::Handle("getplaylist", PlaylistRequest);
		Network::Handle("playlistresponse", PlaylistReponse);
	}

	Playlist::~Playlist()
	{
		Playlist::CurrentPlaylistBuffer.clear();
		Playlist::ReceivedPlaylistBuffer.clear();
	}
}
