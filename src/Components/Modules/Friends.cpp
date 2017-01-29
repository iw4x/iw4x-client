#include "STDInclude.hpp"

namespace Components
{
	unsigned int Friends::CurrentFriend;
	std::recursive_mutex Friends::Mutex;
	std::vector<Friends::Friend> Friends::FriendsList;

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

			IPCHandler::SendWorker("friends", function.SerializeAsString());
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
		std::lock_guard<std::recursive_mutex> _(Friends::Mutex);
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

			buffer[0] = '^';
			buffer[1] = 2;

			// Icon size
			char size = 0x30;
			buffer[2] = size; // Width
			buffer[3] = size; // Height

			// Icon name length
			buffer[4] = static_cast<char>(strlen(rankIcon->name));

			strcat_s(buffer, rankIcon->name);
			strcat_s(buffer, Utils::String::VA(" %i", rank));

			return buffer;
		}
		case 1:
			return Utils::String::VA("%s", user.name.data());

		case 2:
			return "Trickshot Isnipe server";

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
	
	void Friends::PresenceResponse(std::vector<std::string> params)
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

			if (key == "iw4x_status")
			{
				entry->statusName = value;
			}
			else if (key == "iw4x_server")
			{
				entry->server = value;

				// TODO: Query server here?
				if (entry->server.getType() != Game::NA_BAD)
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
		}
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
					Friends::PresenceResponse({ Utils::String::VA("%llx", id.Bits), key, value });
				}
			}
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
			entry.prestige = 0;
			entry.experience = 0;

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

	Friends::Friends()
	{
		Friends::UpdateFriends();

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

		UIFeeder::Add(6.0f, Friends::GetFriendCount, Friends::GetFriendText, Friends::SelectFriend);

		fInterface = IPCHandler::NewInterface("friends");
		fInterface->map("friendsResponse", Friends::FriendsResponse);
		fInterface->map("nameResponse", Friends::NameResponse);
		fInterface->map("presenceResponse", Friends::PresenceResponse);
		fInterface->map("infoResponse", Friends::InfoResponse);
	}

	Friends::~Friends()
	{
		{
			std::lock_guard<std::recursive_mutex> _(Friends::Mutex);
			Friends::FriendsList.clear();
		}
	}
}
