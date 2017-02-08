#include "STDInclude.hpp"

namespace Components
{
	bool Friends::TriggerSort = false;
	bool Friends::TriggerUpdate = false;

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

			return friend1->cleanName.compare(friend2->cleanName);
		});
	}

	void Friends::SortList(bool force)
	{
		if(!force)
		{
			Friends::TriggerSort = true;
			return;
		}

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

		size_t count = Friends::FriendsList.size();
		Friends::FriendsList.clear();
		Friends::FriendsList.reserve(count);

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
		entry->online = Steam::Proxy::SteamFriends->GetFriendPersonaState(user) != 0;
		entry->cleanName = Utils::String::ToLower(Colors::Strip(entry->name));

		std::string guid = Steam::Proxy::SteamFriends->GetFriendRichPresence(user, "iw4x_guid");
		std::string name = Steam::Proxy::SteamFriends->GetFriendRichPresence(user, "iw4x_name");
		std::string experience = Steam::Proxy::SteamFriends->GetFriendRichPresence(user, "iw4x_experience");
		std::string prestige = Steam::Proxy::SteamFriends->GetFriendRichPresence(user, "iw4x_prestige");

		if (!guid.empty()) entry->guid.Bits = strtoull(guid.data(), nullptr, 16);
		if (!name.empty()) entry->playerName = name;
		if (!experience.empty()) entry->experience = atoi(experience.data());
		if (!prestige.empty()) entry->prestige = atoi(prestige.data());

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
		if (entry->server.getType() == Game::NA_LOOPBACK || (entry->server.getType() == Game::NA_IP && entry->server.getIP().full == 0x0100007F)) entry->server.setType(Game::NA_BAD);
		else if (entry->server.getType() != Game::NA_BAD && entry->server != oldAddress)
		{
			Node::AddNode(entry->server);
			Network::SendCommand(entry->server, "getinfo", Utils::Cryptography::Rand::GenerateChallenge());
		}

		Friends::SortList();
	}

	void Friends::UpdateState(bool force)
	{
		if (Dvar::Var("cl_anonymous").get<bool>()) return;

		if(force)
		{
			if (Steam::Proxy::SteamLegacyFriends)
			{
				int state = Steam::Proxy::SteamLegacyFriends->GetPersonaState();
				Steam::Proxy::SteamLegacyFriends->SetPersonaState((state == 1 ? 2 : 1));
			}
		}
		else
		{
			Friends::TriggerUpdate = true;
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

	void Friends::UpdateName()
	{
		Friends::SetPresence("iw4x_name", Steam::SteamFriends()->GetPersonaName());
		Friends::UpdateState();
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
		if (Steam::Proxy::SteamFriends && !Dvar::Var("cl_anonymous").get<bool>())
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

		Proto::Friends::List list;
		list.ParseFromString(Utils::IO::ReadFile("players/friends.dat"));

		for (int i = 0; i < count; ++i)
		{
			SteamID id = Steam::Proxy::SteamFriends->GetFriendByIndex(i, 4);

			Friends::Friend entry;
			entry.userId = id;
			entry.guid.Bits = 0;
			entry.online = false;
			entry.lastTime = 0;
			entry.prestige = 0;
			entry.experience = 0;
			entry.server.setType(Game::NA_BAD);

			for(auto storedFriend : list.friends())
			{
				if(entry.userId.Bits == strtoull(storedFriend.steamid().data(), nullptr, 16))
				{
					entry.playerName = storedFriend.name();
					entry.experience = storedFriend.experience();
					entry.prestige = storedFriend.prestige();
					entry.guid.Bits = strtoull(storedFriend.guid().data(), nullptr, 16);
					break;
				}
			}

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
		{
			if (user.playerName.empty())
			{
				return Utils::String::VA("%s", user.name.data());
			}
			else
			{
				return Utils::String::VA("%s ^7(%s^7)", user.name.data(), user.playerName.data());
			}
		}
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
		Friends::SetPresence("iw4x_playing", Utils::String::VA("%d", Steam::SteamUtils()->GetServerRealTime()));
		Friends::SetPresence("iw4x_guid", Utils::String::VA("%llX", Steam::SteamUser()->GetSteamID().Bits));
	}

	bool Friends::IsOnline(unsigned __int64 timeStamp)
	{
		if (!Steam::Proxy::SteamUtils) return false;
		static const unsigned __int64 duration = std::chrono::duration_cast<std::chrono::seconds>(5min).count();

		return ((Steam::SteamUtils()->GetServerRealTime() - timeStamp) < duration);
	}

	void Friends::StoreFriendsList()
	{
		std::lock_guard<std::recursive_mutex> _(Friends::Mutex);

		Proto::Friends::List list;
		for(auto entry : Friends::FriendsList)
		{
			Proto::Friends::Friend* friendEntry = list.add_friends();

			friendEntry->set_steamid(Utils::String::VA("%llX", entry.userId.Bits));
			friendEntry->set_guid(Utils::String::VA("%llX", entry.guid.Bits));
			friendEntry->set_name(entry.playerName);
			friendEntry->set_experience(entry.experience);
			friendEntry->set_prestige(entry.prestige);
		}

		Utils::IO::WriteFile("players/friends.dat", list.SerializeAsString());
	}

	Friends::Friends()
	{
		if (Dedicated::IsEnabled() ||ZoneBuilder::IsEnabled()) return;
		Dvar::Register<bool>("cl_anonymous", false, Game::DVAR_FLAG_SAVED, "");

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
			static Utils::Time::Interval timeInterval;
			static Utils::Time::Interval sortInterval;
			static Utils::Time::Interval stateInterval;

			if (*reinterpret_cast<bool*>(0x1AD5690)) // LiveStorage_DoWeHaveStats
			{
				Friends::UpdateRank();
			}

			if (timeInterval.elapsed(2min))
			{
				timeInterval.update();
				Friends::UpdateTimeStamp();
				Friends::UpdateState();
			}

			if(stateInterval.elapsed(5s))
			{
				stateInterval.update();

				if(Friends::TriggerUpdate)
				{
					Friends::TriggerUpdate = false;
					Friends::UpdateState(true);
				}
			}

			if(sortInterval.elapsed(3s))
			{
				sortInterval.update();

				if (Friends::TriggerSort)
				{
					Friends::TriggerSort = false;
					Friends::SortList(true);
				}
			}
		});

		UIFeeder::Add(61.0f, Friends::GetFriendCount, Friends::GetFriendText, Friends::SelectFriend);

		QuickPatch::OnShutdown([]()
		{
			Friends::ClearPresence("iw4x_server");
			Friends::ClearPresence("iw4x_playing");

#ifdef DEBUG
			if (Steam::Proxy::SteamFriends)
			{
				Steam::Proxy::SteamFriends->ClearRichPresence();
			}
#endif

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

			if(Dvar::Var("cl_anonymous").get<bool>())
			{
				if (Steam::Proxy::SteamFriends)
				{
					Steam::Proxy::SteamFriends->ClearRichPresence();
				}
			}

			Friends::UpdateTimeStamp();
			Friends::UpdateName();
			Friends::UpdateState();

			Friends::UpdateFriends();
		});
	}

	Friends::~Friends()
	{
		if (Dedicated::IsEnabled() || ZoneBuilder::IsEnabled()) return;

		Friends::StoreFriendsList();

		Steam::Proxy::UnregisterCallback(336);
		Steam::Proxy::UnregisterCallback(304);

		std::lock_guard<std::recursive_mutex> _(Friends::Mutex);
		Friends::FriendsList.clear();

	}
}
