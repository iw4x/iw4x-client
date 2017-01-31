#include "STDInclude.hpp"

namespace Components
{
	int Friends::InitialState;
	unsigned int Friends::CurrentFriend;
	std::recursive_mutex Friends::Mutex;
	std::vector<Friends::Friend> Friends::FriendsList;

	void Friends::SortIndividualList(std::vector<Friends::Friend>* list)
	{
		qsort(list->data(), list->size(), sizeof(Friends::Friend), [](const void* first, const void* second)
		{
			const Friends::Friend* friend1 = static_cast<const Friends::Friend*>(first);
			const Friends::Friend* friend2 = static_cast<const Friends::Friend*>(second);

			std::string name1 = Utils::String::ToLower(Colors::Strip(friend1->name));
			std::string name2 = Utils::String::ToLower(Colors::Strip(friend2->name));

			return name1.compare(name2);
		});
	}

	void Friends::SortList()
	{
		std::lock_guard<std::recursive_mutex> _(Friends::Mutex);

		std::vector<Friends::Friend> connectedList;
		std::vector<Friends::Friend> playingList;
		std::vector<Friends::Friend> onlineList;
		std::vector<Friends::Friend> offlineList;

		// Split up the list
		for(auto entry : Friends::FriendsList)
		{
			if(!entry.online) offlineList.push_back(entry);
			else if(!Friends::IsOnline(entry.lastTime)) onlineList.push_back(entry);
			else if (entry.server.getType() == Game::NA_BAD) playingList.push_back(entry);
			else connectedList.push_back(entry);
		}

		Friends::SortIndividualList(&connectedList);
		Friends::SortIndividualList(&playingList);
		Friends::SortIndividualList(&onlineList);
		Friends::SortIndividualList(&offlineList);

		Friends::FriendsList.clear();

		Utils::Merge(&Friends::FriendsList, connectedList);
		Utils::Merge(&Friends::FriendsList, playingList);
		Utils::Merge(&Friends::FriendsList, onlineList);
		Utils::Merge(&Friends::FriendsList, offlineList);
	}

	void Friends::UpdateUserInfo(SteamID user)
	{
		std::lock_guard<std::recursive_mutex> _(Friends::Mutex);

		auto entry = std::find_if(Friends::FriendsList.begin(), Friends::FriendsList.end(), [user](Friends::Friend entry)
		{
			return (entry.userId.Bits == user.Bits);
		});

		if (entry == Friends::FriendsList.end() || !Steam::Proxy::SteamFriends) return;

		entry->name = Steam::Proxy::SteamFriends->GetFriendPersonaName(user);
		entry->playerName = Steam::Proxy::SteamFriends->GetFriendRichPresence(user, "iw4x_name");
		entry->online = Steam::Proxy::SteamFriends->GetFriendPersonaState(user) != 0;
		entry->guid.Bits = strtoull(Steam::Proxy::SteamFriends->GetFriendRichPresence(user, "iw4x_guid"), nullptr, 16);
		entry->experience = atoi(Steam::Proxy::SteamFriends->GetFriendRichPresence(user, "iw4x_experience"));
		entry->prestige = atoi(Steam::Proxy::SteamFriends->GetFriendRichPresence(user, "iw4x_prestige"));

		std::string server = Steam::Proxy::SteamFriends->GetFriendRichPresence(user, "iw4x_server");
		Network::Address oldAddress = entry->server;

		entry->lastTime = static_cast<unsigned int>(atoi(Steam::Proxy::SteamFriends->GetFriendRichPresence(user, "iw4x_playing")));

		if (server.empty())
		{
			entry->server.setType(Game::NA_BAD);
			entry->serverName.clear();
		}
		else if (entry->server != server)
		{
			entry->server = server;
			entry->serverName.clear();
		}

		// Block localhost
		if (entry->server.getType() == Game::NA_LOOPBACK) entry->server.setType(Game::NA_BAD);
		if (entry->server.getType() != Game::NA_BAD && entry->server != oldAddress)
		{
			Node::AddNode(entry->server);
			Network::SendCommand(entry->server, "getinfo", Utils::Cryptography::Rand::GenerateChallenge());
		}

		Friends::SortList();
	}

	void Friends::UpdateState()
	{
		if(Steam::Proxy::SteamLegacyFriends)
		{
			int state = Steam::Proxy::SteamLegacyFriends->GetPersonaState();
			Steam::Proxy::SteamLegacyFriends->SetPersonaState((state == 1 ? 2 : 1));
		}
	}

	void Friends::UpdateHostname(Network::Address server, std::string hostname)
	{
		std::lock_guard<std::recursive_mutex> _(Friends::Mutex);

		for(auto& entry : Friends::FriendsList)
		{
			if(entry.server == server)
			{
				entry.serverName = hostname;
			}
		}
	}

	void Friends::ClearPresence(std::string key)
	{
		if (Steam::Proxy::SteamFriends)
		{
			Steam::Proxy::SteamFriends->SetRichPresence(key.data(), nullptr);
		}
	}

	void Friends::SetPresence(std::string key, std::string value)
	{
		if (Steam::Proxy::SteamFriends)
		{
			Steam::Proxy::SteamFriends->SetRichPresence(key.data(), value.data());
		}
	}

	void Friends::SetServer()
	{
		Friends::SetPresence("iw4x_server", Network::Address(*Game::connectedHost).getString()); // reinterpret_cast<char*>(0x7ED3F8)
		Friends::UpdateState();
	}

	void Friends::ClearServer()
	{
		Friends::ClearPresence("iw4x_server");
		Friends::UpdateState();
	}

	bool Friends::IsClientInParty(int /*controller*/, int clientNum)
	{
		if (clientNum < 0 || clientNum >= ARRAYSIZE(Dedicated::PlayerGuids)) return false;

		std::lock_guard<std::recursive_mutex> _(Friends::Mutex);
		SteamID guid = Dedicated::PlayerGuids[clientNum];

		for (auto entry : Friends::FriendsList)
		{
			if (entry.guid.Bits == guid.Bits && Friends::IsOnline(entry.lastTime) && entry.online)
			{
				return true;
			}
		}

		return false;
	}

	void Friends::UpdateRank()
	{
		static Utils::Value<int> levelVal;

		int experience = Game::Live_GetXp(0);
		int prestige = Game::Live_GetPrestige(0);
		int level = (experience & 0xFFFFFF) | ((prestige & 0xFF) << 24);

		if(!levelVal.isValid() || levelVal.get() != level)
		{
			levelVal.set(level);

			Friends::SetPresence("iw4x_experience", Utils::String::VA("%d", experience));
			Friends::SetPresence("iw4x_prestige", Utils::String::VA("%d", prestige));
			Friends::UpdateState();
		}
	}

	void Friends::UpdateFriends()
	{
		std::lock_guard<std::recursive_mutex> _(Friends::Mutex);
		if (!Steam::Proxy::SteamFriends) return;

		auto oldFriends = Friends::FriendsList;
		Friends::FriendsList.clear();

		int count = Steam::Proxy::SteamFriends->GetFriendCount(4);

		for (int i = 0; i < count; ++i)
		{
			SteamID id = Steam::Proxy::SteamFriends->GetFriendByIndex(i, 4);

			Friends::Friend entry;
			entry.userId = id;
			entry.online = false;
			entry.lastTime = 0;
			entry.prestige = 0;
			entry.experience = 0;
			entry.server.setType(Game::NA_BAD);

			auto oldEntry = std::find_if(oldFriends.begin(), oldFriends.end(), [id](Friends::Friend entry)
			{
				return (entry.userId.Bits == id.Bits);
			});

			if (oldEntry != oldFriends.end()) entry = *oldEntry;

			Friends::FriendsList.push_back(entry);

			Friends::UpdateUserInfo(id);

			Steam::Proxy::SteamFriends->RequestFriendRichPresence(id);
		}
	}

	unsigned int Friends::GetFriendCount()
	{
		return Friends::FriendsList.size();
	}

	const char* Friends::GetFriendText(unsigned int index, int column)
	{
		std::lock_guard<std::recursive_mutex> _(Friends::Mutex);
		if (index >= Friends::FriendsList.size()) return "";

		auto user = Friends::FriendsList[index];

		switch(column)
		{
		case 0:
		{
			static char buffer[0x100];
			ZeroMemory(buffer, sizeof(buffer));

			Game::Material* rankIcon = nullptr;
			int rank = Game::CL_GetRankForXP(user.experience);
			Game::CL_GetRankIcon(rank, user.prestige, &rankIcon);
			if (!rankIcon) rankIcon = Game::DB_FindXAssetDefaultHeaderInternal(Game::XAssetType::ASSET_TYPE_MATERIAL).material;

			buffer[0] = '^';
			buffer[1] = 2;

			// Icon size
			char size = 0x30;
			buffer[2] = size; // Width
			buffer[3] = size; // Height

			// Icon name length
			buffer[4] = static_cast<char>(strlen(rankIcon->name));

			strcat_s(buffer, rankIcon->name);
			strcat_s(buffer, Utils::String::VA(" %i", (rank + 1)));

			return buffer;
		}
		case 1:
			return Utils::String::VA("%s", user.name.data());

		case 2:
		{		
			if (!user.online) return "Offline";
			if (!Friends::IsOnline(user.lastTime)) return "Online";
			if (user.server.getType() == Game::NA_BAD) return "Playing IW4x";
			if (user.serverName.empty()) return Utils::String::VA("Playing on %s", user.server.getCString());
			return Utils::String::VA("Playing on %s", user.serverName.data());
		}

		default:
			break;
		}

		return "";
	}

	void Friends::SelectFriend(unsigned int index)
	{
		std::lock_guard<std::recursive_mutex> _(Friends::Mutex);
		if (index >= Friends::FriendsList.size()) return;

		Friends::CurrentFriend = index;
	}

	__declspec(naked) void Friends::DisconnectStub()
	{
		__asm
		{
			pushad
			call Friends::ClearServer
			popad

			push 467CC0h
			retn
		}
	}

	void Friends::UpdateTimeStamp()
	{
		Friends::SetPresence("iw4x_playing", Utils::String::VA("%d", Steam::Proxy::SteamUtils->GetServerRealTime()));
	}

	bool Friends::IsOnline(unsigned __int64 timeStamp)
	{
		if (!Steam::Proxy::SteamUtils) return false;
		static const unsigned __int64 duration = std::chrono::duration_cast<std::chrono::seconds>(5min).count();

		return ((Steam::Proxy::SteamUtils->GetServerRealTime() - timeStamp) < duration);
	}

	Friends::Friends()
	{
		// Callback to update user information
		Steam::Proxy::RegisterCallback(336, [](void* data)
		{
			Friends::FriendRichPresenceUpdate* update = static_cast<Friends::FriendRichPresenceUpdate*>(data);
			Friends::UpdateUserInfo(update->m_steamIDFriend);
		});

		// Persona state has changed
		Steam::Proxy::RegisterCallback(304, [](void* data)
		{
			Friends::PersonaStateChange* state = static_cast<Friends::PersonaStateChange*>(data);
			if (Steam::Proxy::SteamFriends) Steam::Proxy::SteamFriends->RequestFriendRichPresence(state->m_ulSteamID);
		});

		// Update state when connecting/disconnecting
		Utils::Hook(0x403582, Friends::DisconnectStub, HOOK_CALL).install()->quick();
		Utils::Hook(0x4CD023, Friends::SetServer, HOOK_JUMP).install()->quick();

		// Show blue icons on the minimap
		Utils::Hook(0x493130, Friends::IsClientInParty, HOOK_JUMP).install()->quick();

		UIScript::Add("LoadFriends", [](UIScript::Token)
		{
			Friends::UpdateFriends();
		});

		UIScript::Add("JoinFriend", [](UIScript::Token)
		{
			std::lock_guard<std::recursive_mutex> _(Friends::Mutex);
			if (Friends::CurrentFriend >= Friends::FriendsList.size()) return;

			auto& user = Friends::FriendsList[Friends::CurrentFriend];

			if(user.online && user.server.getType() != Game::NA_BAD)
			{
				Party::Connect(user.server);
			}
			else
			{
				Command::Execute("snd_playLocal exit_prestige", false);
			}
		});

		QuickPatch::OnFrame([]()
		{
			if(*reinterpret_cast<bool*>(0x1AD5690)) // LiveStorage_DoWeHaveStats
			{
				Friends::UpdateRank();
			}
		});

		UIFeeder::Add(6.0f, Friends::GetFriendCount, Friends::GetFriendText, Friends::SelectFriend);

		QuickPatch::OnShutdown([]()
		{
			Friends::ClearPresence("iw4x_server");
			Friends::ClearPresence("iw4x_playing");

			//Steam::Proxy::SteamFriends->ClearRichPresence();

			if(Steam::Proxy::SteamLegacyFriends)
			{
				Steam::Proxy::SteamLegacyFriends->SetPersonaState(Friends::InitialState);
			}
		});

		QuickPatch::Once([]()
		{
			if (Steam::Proxy::SteamLegacyFriends)
			{
				Friends::InitialState = Steam::Proxy::SteamLegacyFriends->GetPersonaState();
			}

			Friends::SetPresence("iw4x_guid", Utils::String::VA("%llX", Steam::SteamUser()->GetSteamID().Bits));
			//Friends::UpdateState(); // Don't update state yet, stats will do that
			Friends::UpdateTimeStamp();
			Friends::UpdateFriends();
		});

		QuickPatch::OnFrame([]()
		{
			static Utils::Time::Interval interval;

			if(interval.elapsed(2min))
			{
				interval.update();
				Friends::UpdateTimeStamp();
				Friends::UpdateState();
			}
		});
	}

	Friends::~Friends()
	{
		Steam::Proxy::UnregisterCallback(336);
		Steam::Proxy::UnregisterCallback(304);

		{
			std::lock_guard<std::recursive_mutex> _(Friends::Mutex);
			Friends::FriendsList.clear();
		}
	}
}
