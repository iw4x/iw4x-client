#include "STDInclude.hpp"

namespace Components
{
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
			if(entry.online)
			{
				if (entry.playing)
				{
					if (entry.server.getType() == Game::NA_BAD)
					{
						playingList.push_back(entry);
					}
					else
					{
						connectedList.push_back(entry);
					}
				}
				else
				{
					onlineList.push_back(entry);
				}
			}
			else
			{
				offlineList.push_back(entry);
			}
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
			Proto::IPC::Function function;

			function.set_name("getInfo");
			*function.add_params() = Utils::String::VA("%llx", user.Bits);

			*function.add_params() = "name";
			*function.add_params() = "state";
			*function.add_params() = "iw4x_name";
			*function.add_params() = "iw4x_status";
			*function.add_params() = "iw4x_rank";
			*function.add_params() = "iw4x_server";
			*function.add_params() = "iw4x_playing";
			*function.add_params() = "iw4x_guid";

			IPCHandler::SendWorker("friends", function.SerializeAsString());
	}

	void Friends::UpdateState()
	{
		Proto::IPC::Function function;
		function.set_name("notifyChange");
		IPCHandler::SendWorker("friends", function.SerializeAsString());
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
		Proto::IPC::Function function;
		function.set_name("setPresence");
		*function.add_params() = key;

		IPCHandler::SendWorker("friends", function.SerializeAsString());
	}

	void Friends::SetPresence(std::string key, std::string value)
	{
		Proto::IPC::Function function;
		function.set_name("setPresence");
		*function.add_params() = key;
		*function.add_params() = value;

		IPCHandler::SendWorker("friends", function.SerializeAsString());
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
			if (entry.guid.Bits == guid.Bits && entry.playing && entry.online)
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

			Friends::SetPresence("iw4x_rank", std::string(reinterpret_cast<char*>(&level), 4));
			Friends::UpdateState();
		}
	}

	void Friends::UpdateFriends()
	{
		Proto::IPC::Function function;
		function.set_name("getFriends");
		*function.add_params() = Utils::String::VA("%d", 4);

		IPCHandler::SendWorker("friends", function.SerializeAsString());
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
			if(user.online)
			{
				if (user.playing)
				{
					if (user.server.getType() != Game::NA_BAD)
					{
						if (user.serverName.empty())
						{
							return Utils::String::VA("Playing on %s", user.server.getCString());
						}
						else
						{
							return Utils::String::VA("Playing on %s", user.serverName.data());
						}
					}
					else
					{
						return "Playing IW4x";
					}
				}
				else
				{
					return "Online";
				}
			}
			else
			{
				return "Offline";
			}
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

	void Friends::NameResponse(std::vector<std::string> params)
	{
		if (params.size() >= 2)
		{
			std::lock_guard<std::recursive_mutex> _(Friends::Mutex);

			SteamID id;
			id.Bits = strtoull(params[0].data(), nullptr, 16);

			for(auto& entry : Friends::FriendsList)
			{
				if(entry.userId.Bits == id.Bits)
				{
					entry.name = params[1];
					break;
				}
			}
		}
	}

	void Friends::ParsePresence(std::vector<std::string> params, bool sort)
	{
		if (params.size() >= 3)
		{
			std::lock_guard<std::recursive_mutex> _(Friends::Mutex);

			SteamID id;
			id.Bits = strtoull(params[0].data(), nullptr, 16);
			std::string key = params[1];
			std::string value = params[2];

			auto entry = std::find_if(Friends::FriendsList.begin(), Friends::FriendsList.end(), [id](Friends::Friend entry)
			{
				return (entry.userId.Bits == id.Bits);
			});

			if (entry == Friends::FriendsList.end()) return;

			if (key == "iw4x_name")
			{
				entry->playerName = value;
			}
			else if (key == "iw4x_playing")
			{
				entry->playing = atoi(value.data()) == 1;
			}
			else if(key == "iw4x_guid")
			{
				entry->guid.Bits = strtoull(value.data(), nullptr, 16);
			}
			else if (key == "iw4x_server")
			{
				Network::Address oldAddress = entry->server;

				if (value.empty())
				{
					entry->server.setType(Game::NA_BAD);
					entry->serverName.clear();
				}
				else if (entry->server != value)
				{
					entry->server = value;
					entry->serverName.clear();
				}

				if (entry->server.getType() != Game::NA_BAD && entry->server != oldAddress)
				{
					Node::AddNode(entry->server);
					Network::SendCommand(entry->server, "getinfo", Utils::Cryptography::Rand::GenerateChallenge());
				}
			}
			else if (key == "iw4x_rank")
			{
				if (value.size() == 4)
				{
					int data = *reinterpret_cast<int*>(const_cast<char*>(value.data()));

					entry->experience = data & 0xFFFFFF;
					entry->prestige = (data >> 24) & 0xFF;
				}
			}

			if (sort) Friends::SortList();
		}
	}
	
	void Friends::PresenceResponse(std::vector<std::string> params)
	{
		Friends::ParsePresence(params, true);
	}

	void Friends::InfoResponse(std::vector<std::string> params)
	{
		if (params.size() >= 1)
		{
			std::lock_guard<std::recursive_mutex> _(Friends::Mutex);

			SteamID id;
			id.Bits = strtoull(params[0].data(), nullptr, 16);

			auto entry = std::find_if(Friends::FriendsList.begin(), Friends::FriendsList.end(), [id](Friends::Friend entry)
			{
				return (entry.userId.Bits == id.Bits);
			});

			if (entry == Friends::FriendsList.end()) return;

			for(unsigned int i = 1; i < params.size(); i += 2)
			{
				if ((i + 1) >= params.size()) break;
				std::string key = params[i];
				std::string value = params[i + 1];

				if(key == "name")
				{
					entry->name = value;
				}
				else if(key == "state")
				{
					entry->online = atoi(value.data()) != 0;
				}
				else
				{
					Friends::ParsePresence({ Utils::String::VA("%llx", id.Bits), key, value }, false);
				}
			}

			Friends::SortList();
		}
	}

	void Friends::FriendsResponse(std::vector<std::string> params)
	{
		std::lock_guard<std::recursive_mutex> _(Friends::Mutex);

		auto oldFriends = Friends::FriendsList;
		Friends::FriendsList.clear();

		for (auto param : params)
		{
			SteamID id;
			id.Bits = strtoull(param.data(), nullptr, 16);

			Friends::Friend entry;
			entry.userId = id;
			entry.online = false;
			entry.playing = false;
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

			Proto::IPC::Function function;
			function.set_name("requestPresence");
			*function.add_params() = Utils::String::VA("%llx", id.Bits);
			IPCHandler::SendWorker("friends", function.SerializeAsString());
		}
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

	Friends::Friends()
	{
		Friends::UpdateFriends();

		// Update state when connecting/disconnecting
		Utils::Hook(0x403582, Friends::DisconnectStub, HOOK_CALL).install()->quick();
		Utils::Hook(0x4CD023, Friends::SetServer, HOOK_JUMP).install()->quick();

		// Show blue icons on the minimap
		Utils::Hook(0x493130, Friends::IsClientInParty, HOOK_JUMP).install()->quick();

		auto fInterface = IPCHandler::NewInterface("steamCallbacks");

		// Callback to update user information
		fInterface->map("336", [](std::vector<std::string> params)
		{
			if (params.size() >= 1 && params[0].size() == sizeof(Friends::FriendRichPresenceUpdate))
			{
				const Friends::FriendRichPresenceUpdate* update = reinterpret_cast<const Friends::FriendRichPresenceUpdate*>(params[0].data());
				Friends::UpdateUserInfo(update->m_steamIDFriend);
			}
		});

		// Persona state has changed
		fInterface->map("304", [](std::vector<std::string> params)
		{
			if(params.size() >= 1 && params[0].size() == sizeof(Friends::PersonaStateChange))
			{
				const Friends::PersonaStateChange* state = reinterpret_cast<const Friends::PersonaStateChange*>(params[0].data());

				Proto::IPC::Function function;
				function.set_name("requestPresence");
				*function.add_params() = Utils::String::VA("%llx", state->m_ulSteamID.Bits);
				IPCHandler::SendWorker("friends", function.SerializeAsString());
			}
		});

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

		fInterface = IPCHandler::NewInterface("friends");
		fInterface->map("friendsResponse", Friends::FriendsResponse);
		fInterface->map("nameResponse", Friends::NameResponse);
		fInterface->map("presenceResponse", Friends::PresenceResponse);
		fInterface->map("infoResponse", Friends::InfoResponse);

		Friends::SetPresence("iw4x_playing", "1");
		Friends::SetPresence("iw4x_guid", Utils::String::VA("%llX", Steam::SteamUser()->GetSteamID().Bits));
	}

	Friends::~Friends()
	{
		{
			std::lock_guard<std::recursive_mutex> _(Friends::Mutex);
			Friends::FriendsList.clear();
		}
	}
}
