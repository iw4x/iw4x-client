#include "STDInclude.hpp"

namespace Components
{
	std::string Playlist::CurrentPlaylistBuffer;
	std::string Playlist::ReceivedPlaylistBuffer;

	void Playlist::LoadPlaylist()
	{
		// Check if playlist already loaded
		if (Utils::Hook::Get<bool>(0x1AD3680)) return;

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

		// Split playlist data
		unsigned int maxPacketSize = 1000;
		unsigned int maxBytes = Playlist::CurrentPlaylistBuffer.size();

		for (unsigned int i = 0; i < maxBytes; i += maxPacketSize)
		{
			unsigned int sendBytes = min(maxPacketSize, maxBytes - i);
			unsigned int sentBytes = i + sendBytes;

			std::string data;
			data.append(reinterpret_cast<char*>(&sentBytes), 4); // Sent bytes
			data.append(reinterpret_cast<char*>(&maxBytes), 4); // Max bytes

			data.append(Playlist::CurrentPlaylistBuffer.data() + i, sendBytes);

			Network::SendRaw(address, std::string("playlistresponse\n") + data);
		}
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
					unsigned int sentBytes = *(unsigned int*)(data.data() + 0);
					unsigned int maxBytes = *(unsigned int*)(data.data() + 4);

					// Clear current buffer, if we receive a new packet
					if (data.size() - 8 == sentBytes) Playlist::ReceivedPlaylistBuffer.clear();

					// Append received data
					Playlist::ReceivedPlaylistBuffer.append(data.data() + 8, data.size() - 8);

					if (Playlist::ReceivedPlaylistBuffer.size() != sentBytes)
					{
						Party::PlaylistError(Utils::VA("Received playlist data, but it seems invalid: %d != %d", sentBytes, Playlist::ReceivedPlaylistBuffer.size()));
						Playlist::ReceivedPlaylistBuffer.clear();
						return;
					}
					else
					{
						Logger::Print("Received playlist data: %d/%d (%d%%)\n", sentBytes, maxBytes, ((100 * sentBytes) / maxBytes));
					}

					if (Playlist::ReceivedPlaylistBuffer.size() == maxBytes)
					{
						Logger::Print("Received playlist, loading and continuing connection...\n");
						Game::Live_ParsePlaylists(Playlist::ReceivedPlaylistBuffer.data());
						Party::PlaylistContinue();

						Playlist::ReceivedPlaylistBuffer.clear();
					}
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
