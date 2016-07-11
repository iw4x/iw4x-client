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

		Proto::Party::Playlist list;
		list.set_hash(Utils::Cryptography::JenkinsOneAtATime::Compute(compressedList));
		list.set_buffer(compressedList);

		Network::SendCommand(address, "playlistResponse", list.SerializeAsString());
	}

	void Playlist::PlaylistReponse(Network::Address address, std::string data)
	{
		if (Party::PlaylistAwaiting())
		{
			if (address == Party::Target())
			{
				Proto::Party::Playlist list;

				if (!list.ParseFromString(data))
				{
					Party::PlaylistError(fmt::sprintf("Received playlist response from %s, but it is invalid.", address.GetCString()));
					Playlist::ReceivedPlaylistBuffer.clear();
					return;
				}
				else
				{
					// Generate buffer and hash
					std::string compressedData(list.buffer());
					unsigned int hash = Utils::Cryptography::JenkinsOneAtATime::Compute(compressedData);

					//Validate hashes
					if (hash != list.hash())
					{
						Party::PlaylistError(fmt::sprintf("Received playlist response from %s, but the checksum did not match (%X != %X).", address.GetCString(), list.hash(), hash));
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

		Network::Handle("getPlaylist", PlaylistRequest);
		Network::Handle("playlistResponse", PlaylistReponse);
	}

	Playlist::~Playlist()
	{
		Playlist::CurrentPlaylistBuffer.clear();
		Playlist::ReceivedPlaylistBuffer.clear();
	}
}
