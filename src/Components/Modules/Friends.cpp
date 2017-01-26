#include "STDInclude.hpp"

namespace Components
{
	unsigned int Friends::CurrentFriend;
	std::recursive_mutex Friends::Mutex;
	std::vector<Friends::Friend> Friends::FriendsList;

	void Friends::UpdateUserInfo(SteamID user)
	{
		if (!Steam::Proxy::SteamFriends) return;
		std::lock_guard<std::recursive_mutex> _(Friends::Mutex);

		Friends::Friend userInfo;

		auto i = std::find_if(Friends::FriendsList.begin(), Friends::FriendsList.end(), [user] (Friends::Friend entry)
		{
			return (entry.userId.Bits == user.Bits);
		});

		if(i != Friends::FriendsList.end())
		{
			userInfo = *i;
		}

		userInfo.userId = user;
		userInfo.online = Steam::Proxy::SteamFriends->GetFriendPersonaState(user) != 0;
		userInfo.name = Steam::Proxy::SteamFriends->GetFriendPersonaName(user);
		userInfo.statusName = Steam::Proxy::SteamFriends->GetFriendRichPresence(user, "iw4x_status");
		userInfo.prestige = Utils::Cryptography::Rand::GenerateInt() % (10 + 1);
		userInfo.experience = Utils::Cryptography::Rand::GenerateInt() % (2516000 + 1);

		//if (!userInfo.online) return;

		if (i != Friends::FriendsList.end())
		{
			*i = userInfo;
		}
		else
		{
			Friends::FriendsList.push_back(userInfo);
		}

		qsort(Friends::FriendsList.data(), Friends::FriendsList.size(), sizeof(Friends::Friend), [](const void* first, const void* second)
		{
			const Friends::Friend* friend1 = static_cast<const Friends::Friend*>(first);
			const Friends::Friend* friend2 = static_cast<const Friends::Friend*>(second);

			std::string name1 = Utils::String::ToLower(Colors::Strip(friend1->name));
			std::string name2 = Utils::String::ToLower(Colors::Strip(friend2->name));

			return name1.compare(name2);
		});
	}

	void Friends::UpdateFriends()
	{
		if (!Steam::Proxy::SteamFriends) return;
		std::lock_guard<std::recursive_mutex> _(Friends::Mutex);

		auto listCopy = Friends::FriendsList;
		Friends::FriendsList.clear();

		int count = Steam::Proxy::SteamFriends->GetFriendCount(4);
		Friends::FriendsList.reserve(count);

		for(int i = 0; i < count; ++i)
		{
			SteamID friendId = Steam::Proxy::SteamFriends->GetFriendByIndex(i, 4);
			//if(!Steam::Proxy::SteamFriends->GetFriendPersonaState(friendId)) continue; // Offline

			auto entry = std::find_if(listCopy.begin(), listCopy.end(), [friendId](Friends::Friend entry)
			{
				return (entry.userId.Bits == friendId.Bits);
			});

			if (entry != listCopy.end())
			{
				Friends::FriendsList.push_back(*entry);
			}

			Friends::UpdateUserInfo(friendId);
			Steam::Proxy::SteamFriends->RequestFriendRichPresence(friendId);
		}
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
			if(Steam::Proxy::SteamFriends) Steam::Proxy::SteamFriends->RequestFriendRichPresence(state->m_ulSteamID);
		});

		UIScript::Add("LoadFriends", [](UIScript::Token)
		{
			Friends::UpdateFriends();
		});

		UIFeeder::Add(6.0f, Friends::GetFriendCount, Friends::GetFriendText, Friends::SelectFriend);
	}

	Friends::~Friends()
	{
		Steam::Proxy::UnregisterCallback(304);
		Steam::Proxy::UnregisterCallback(336);

		if (Steam::Proxy::SteamFriends)
		{
			Steam::Proxy::SteamFriends->SetRichPresence("iw4x_status", nullptr);
		}

		{
			std::lock_guard<std::recursive_mutex> _(Friends::Mutex);
			Friends::FriendsList.clear();
		}
	}
}
