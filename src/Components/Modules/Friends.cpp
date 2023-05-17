#include <STDInclude.hpp>

#pragma warning(push)
#pragma warning(disable: 4100)
#include <proto/friends.pb.h>
#pragma warning(pop)

#include "Events.hpp"
#include "Friends.hpp"
#include "Materials.hpp"
#include "Node.hpp"
#include "Party.hpp"
#include "TextRenderer.hpp"
#include "Toast.hpp"
#include "UIFeeder.hpp"

namespace Components
{
	bool Friends::LoggedOn = false;
	bool Friends::TriggerSort = false;
	bool Friends::TriggerUpdate = false;

	int Friends::InitialState;
	unsigned int Friends::CurrentFriend;
	std::recursive_mutex Friends::Mutex;
	std::vector<Friends::Friend> Friends::FriendsList;

	Dvar::Var Friends::UIStreamFriendly;
	Dvar::Var Friends::CLAnonymous;
	Dvar::Var Friends::CLNotifyFriendState;

	void Friends::SortIndividualList(std::vector<Friends::Friend>* list)
	{
		std::stable_sort(list->begin(), list->end(), [](Friends::Friend const& friend1, Friends::Friend const& friend2)
		{
			return friend1.cleanName.compare(friend2.cleanName) < 0;
		});
	}

	void Friends::SortList(bool force)
	{
		if (!force)
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
		for (auto entry : Friends::FriendsList)
		{
			if (!entry.online) offlineList.push_back(entry);
			else if (!Friends::IsOnline(entry.lastTime)) onlineList.push_back(entry);
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
			return (entry.userId.bits == user.bits);
		});

		if (entry == Friends::FriendsList.end() || !Steam::Proxy::SteamFriends) return;

		entry->name = Steam::Proxy::SteamFriends->GetFriendPersonaName(user);
		entry->online = Steam::Proxy::SteamFriends->GetFriendPersonaState(user) != 0;
		entry->cleanName = Utils::String::ToLower(TextRenderer::StripColors(entry->name));

		std::string guid = Friends::GetPresence(user, "iw4x_guid");
		std::string name = Friends::GetPresence(user, "iw4x_name");
		std::string experience = Friends::GetPresence(user, "iw4x_experience");
		std::string prestige = Friends::GetPresence(user, "iw4x_prestige");

		if (!guid.empty()) entry->guid.bits = strtoull(guid.data(), nullptr, 16);
		if (!name.empty()) entry->playerName = name;
		if (!experience.empty()) entry->experience = atoi(experience.data());
		if (!prestige.empty()) entry->prestige = atoi(prestige.data());

		std::string server = Friends::GetPresence(user, "iw4x_server");
		Network::Address oldAddress = entry->server;

		bool gotOnline = Friends::IsOnline(entry->lastTime);
		entry->lastTime = static_cast<unsigned int>(atoi(Friends::GetPresence(user, "iw4x_playing").data()));
		gotOnline = !gotOnline && Friends::IsOnline(entry->lastTime);

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
			Node::Add(entry->server);
			Network::SendCommand(entry->server, "getinfo", Utils::Cryptography::Rand::GenerateChallenge());
		}

		Friends::SortList();

		const auto notify = Friends::CLNotifyFriendState.get<bool>();
		if (gotOnline && (!notify || (notify && !Game::CL_IsCgameInitialized())) && !Friends::UIStreamFriendly.get<bool>())
		{
			Game::Material* material = Friends::CreateAvatar(user);
			Toast::Show(material, entry->name, "is playing IW4x", 3000, [material]()
			{
				Materials::Delete(material, true);
			});
		}
	}

	void Friends::UpdateState()
	{
		if (Friends::CLAnonymous.get<bool>() || Friends::IsInvisible() || !Steam::Enabled())
		{
			return;
		}

		Friends::TriggerUpdate = true;
	}

	void Friends::UpdateServer(Network::Address server, const std::string& hostname, const std::string& mapname)
	{
		std::lock_guard<std::recursive_mutex> _(Friends::Mutex);

		for (auto& entry : Friends::FriendsList)
		{
			if (entry.server == server)
			{
				entry.serverName = hostname;
				entry.mapname = mapname;
			}
		}
	}

	void Friends::UpdateName()
	{
		Friends::SetPresence("iw4x_name", Steam::SteamFriends()->GetPersonaName());
		Friends::UpdateState();
	}

	std::vector<int> Friends::GetAppIdList()
	{
		std::vector<int> ids;

		const auto addId = [&](int id)
		{
			if (std::find(ids.begin(), ids.end(), id) == ids.end())
			{
				ids.push_back(id);
			}
		};

		addId(0);
		addId(10190);
		addId(480);
		addId(Steam::Proxy::AppId);

		if (Steam::Proxy::SteamUtils)
		{
			addId(Steam::Proxy::SteamUtils->GetAppID());
		}

		if (Steam::Proxy::SteamFriends)
		{
			std::lock_guard<std::recursive_mutex> _(Friends::Mutex);

			const auto modId = *reinterpret_cast<const unsigned int*>("IW4x") | 0x80000000;

			// Split up the list
			for (const auto& entry : Friends::FriendsList)
			{
				Steam::FriendGameInfo info;
				if (Steam::Proxy::SteamFriends->GetFriendGamePlayed(entry.userId, &info) && info.m_gameID.modID == modId)
				{
					addId(info.m_gameID.appID);
				}
			}
		}

		return ids;
	}

	void Friends::SetRawPresence(const char* key, const char* value)
	{
		if (Steam::Proxy::ClientFriends)
		{
			// Set the presence for all possible apps that IW4x might have to interact with.
			// GetFriendRichPresence only reads values for the app that we are running,
			// therefore our friends (and we as well) have to set the presence for those apps.
			auto appIds = Friends::GetAppIdList();

			for (auto id : appIds)
			{
				Steam::Proxy::ClientFriends.invoke<void>("SetRichPresence", id, key, value);
			}
		}
	}

	void Friends::ClearPresence(const std::string& key)
	{
		if (Steam::Proxy::ClientFriends && Steam::Proxy::SteamUtils)
		{
			Friends::SetRawPresence(key.data(), nullptr);
		}
	}

	void Friends::SetPresence(const std::string& key, const std::string& value)
	{
		if (Steam::Proxy::ClientFriends && Steam::Proxy::SteamUtils && !Friends::CLAnonymous.get<bool>() && !Friends::IsInvisible() && Steam::Enabled())
		{
			Friends::SetRawPresence(key.data(), value.data());
		}
	}

	void Friends::RequestPresence(SteamID user)
	{
		if (Steam::Proxy::ClientFriends)
		{
			Steam::Proxy::ClientFriends.invoke<void>("RequestFriendRichPresence", Friends::GetGame(user), user);
		}
	}

	std::string Friends::GetPresence(SteamID user, const std::string& key)
	{
		if (!Steam::Proxy::ClientFriends || !Steam::Proxy::SteamUtils) return "";

		std::string result = Steam::Proxy::ClientFriends.invoke<const char*>("GetFriendRichPresence", Friends::GetGame(user), user, key.data());
		return result;
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
		SteamID guid = Dedicated::PlayerGuids[clientNum][0];

		for (auto entry : Friends::FriendsList)
		{
			if (entry.guid.bits == guid.bits && Friends::IsOnline(entry.lastTime) && entry.online)
			{
				return true;
			}
		}

		return false;
	}

	void Friends::UpdateRank()
	{
		static std::optional<int> levelVal;

		int experience = Game::Live_GetXp(0);
		int prestige = Game::Live_GetPrestige(0);
		int level = (experience & 0xFFFFFF) | ((prestige & 0xFF) << 24);

		if (!levelVal.has_value() || levelVal.value() != level)
		{
			levelVal.emplace(level);

			Friends::SetPresence("iw4x_experience", Utils::String::VA("%d", experience));
			Friends::SetPresence("iw4x_prestige", Utils::String::VA("%d", prestige));
			Friends::UpdateState();
		}
	}

	void Friends::UpdateFriends()
	{
		std::lock_guard<std::recursive_mutex> _(Friends::Mutex);

		Friends::LoggedOn = (Steam::Proxy::SteamUser_ && Steam::Proxy::SteamUser_->LoggedOn());
		if (!Steam::Proxy::SteamFriends) return;

		Game::UI_UpdateArenas();

		int count = Steam::Proxy::SteamFriends->GetFriendCount(4);

		Proto::Friends::List list;
		list.ParseFromString(Utils::IO::ReadFile("players/friends.dat"));

		std::vector<Friends::Friend> steamFriends;

		for (int i = 0; i < count; ++i)
		{
			SteamID id = Steam::Proxy::SteamFriends->GetFriendByIndex(i, 4);

			Friends::Friend entry;
			entry.userId = id;
			entry.guid.bits = 0;
			entry.online = false;
			entry.lastTime = 0;
			entry.prestige = 0;
			entry.experience = 0;
			entry.server.setType(Game::NA_BAD);

			for (auto storedFriend : list.friends())
			{
				if (entry.userId.bits == strtoull(storedFriend.steamid().data(), nullptr, 16))
				{
					entry.playerName = storedFriend.name();
					entry.experience = storedFriend.experience();
					entry.prestige = storedFriend.prestige();
					entry.guid.bits = strtoull(storedFriend.guid().data(), nullptr, 16);
					break;
				}
			}

			auto oldEntry = std::find_if(Friends::FriendsList.begin(), Friends::FriendsList.end(), [id](Friends::Friend entry)
			{
				return (entry.userId.bits == id.bits);
			});

			if (oldEntry != Friends::FriendsList.end()) entry = *oldEntry;
			else Friends::FriendsList.push_back(entry);

			steamFriends.push_back(entry);
		}

		for (auto i = Friends::FriendsList.begin(); i != Friends::FriendsList.end();)
		{
			SteamID id = i->userId;

			auto oldEntry = std::find_if(steamFriends.begin(), steamFriends.end(), [id](Friends::Friend entry)
			{
				return (entry.userId.bits == id.bits);
			});

			if (oldEntry == steamFriends.end())
			{
				i = Friends::FriendsList.erase(i);
			}
			else
			{
				*i = *oldEntry;
				++i;

				Friends::UpdateUserInfo(id);
				Friends::RequestPresence(id);
			}
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

		switch (column)
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
			buffer[4] = static_cast<char>(strlen(rankIcon->info.name));

			strcat_s(buffer, rankIcon->info.name);
			strcat_s(buffer, Utils::String::VA(" %i", (rank + 1)));

			return buffer;
		}
		case 1:
		{
			if (user.playerName.empty())
			{
				return Utils::String::VA("%s", user.name.data());
			}

			if (user.name == user.playerName)
			{
				return Utils::String::VA("%s", user.name.data());
			}

			return Utils::String::VA("%s ^7(%s^7)", user.name.data(), user.playerName.data());
		}
		case 2:
		{
			if (!user.online) return "Offline";
			if (!Friends::IsOnline(user.lastTime)) return "Online";
			if (user.server.getType() == Game::NA_BAD) return "Playing IW4x";
			if (user.serverName.empty()) return Utils::String::VA("Playing on %s", user.server.getCString());
			return Utils::String::VA("Playing %s on %s", Localization::LocalizeMapName(user.mapname.data()), user.serverName.data());
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

	int Friends::GetGame(SteamID user)
	{
		int appId = 0;

		Steam::FriendGameInfo info;
		if (Steam::Proxy::SteamFriends && Steam::Proxy::SteamFriends->GetFriendGamePlayed(user, &info))
		{
			appId = info.m_gameID.appID;
		}

		return appId;
	}

	bool Friends::IsInvisible()
	{
		return Friends::InitialState == 7;
	}

	void Friends::UpdateTimeStamp()
	{
		Friends::SetPresence("iw4x_playing", Utils::String::VA("%d", Steam::SteamUtils()->GetServerRealTime()));
		Friends::SetPresence("iw4x_guid", Utils::String::VA("%llX", Steam::SteamUser()->GetSteamID().bits));
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

		// Only store our cache if we are logged in, otherwise it might be invalid
		if (!Friends::LoggedOn) return;

		Proto::Friends::List list;
		for (auto entry : Friends::FriendsList)
		{
			Proto::Friends::Friend* friendEntry = list.add_friends();

			friendEntry->set_steamid(Utils::String::VA("%llX", entry.userId.bits));
			friendEntry->set_guid(Utils::String::VA("%llX", entry.guid.bits));
			friendEntry->set_name(entry.playerName);
			friendEntry->set_experience(entry.experience);
			friendEntry->set_prestige(entry.prestige);
		}

		Utils::IO::WriteFile("players/friends.dat", list.SerializeAsString());
	}

	Game::Material* Friends::CreateAvatar(SteamID user)
	{
		if (!Steam::Proxy::SteamUtils || !Steam::Proxy::SteamFriends) return nullptr;

		int index = Steam::Proxy::SteamFriends->GetMediumFriendAvatar(user);

		unsigned int width, height;
		Steam::Proxy::SteamUtils->GetImageSize(index, &width, &height);

		Game::GfxImage* image = Materials::CreateImage(Utils::String::VA("texture_%llX", user.bits), width, height, 1, 0x1000003, D3DFMT_A8R8G8B8);

		D3DLOCKED_RECT lockedRect;
		image->texture.map->LockRect(0, &lockedRect, nullptr, 0);

		unsigned char* buffer = static_cast<unsigned char*>(lockedRect.pBits);
		Steam::Proxy::SteamUtils->GetImageRGBA(index, buffer, width * height * 4);

		// Swap red and blue channel
		for (unsigned int i = 0; i < width * height * 4; i += 4)
		{
			std::swap(buffer[i + 0], buffer[i + 2]);
		}

		// Steam rounds the corners and somehow fuck up the pixels there
		buffer[3] = 0;                                                // top-left
		buffer[(width - 1) * 4 + 3] = 0;                              // top-right
		buffer[((height - 1) * width * 4) + 3] = 0;                   // bottom-left
		buffer[((height - 1) * width * 4) + (width - 1) * 4 + 3] = 0; // bottom-right

		image->texture.map->UnlockRect(0);

		return Materials::Create(Utils::String::VA("avatar_%llX", user.bits), image);
	}

	Friends::Friends()
	{
		Friends::LoggedOn = false;

		if (Dedicated::IsEnabled() || ZoneBuilder::IsEnabled()) return;

		Friends::UIStreamFriendly = Dvar::Register<bool>("ui_streamFriendly", false, Game::DVAR_ARCHIVE, "Stream friendly UI");
		Friends::CLAnonymous = Dvar::Register<bool>("cl_anonymous", false, Game::DVAR_ARCHIVE, "Enable invisible mode for Steam");
		Friends::CLNotifyFriendState = Dvar::Register<bool>("cl_notifyFriendState", true, Game::DVAR_ARCHIVE, "Update friends about current game status");

		// Hook Live_ShowFriendsList
		Utils::Hook(0x4D6C70, []()
		{
			Command::Execute("openmenu popup_friends", true);
		}, HOOK_JUMP).install()->quick();

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
			Friends::RequestPresence(state->m_ulSteamID);
		});

		// Update state when connecting/disconnecting
		Events::OnSteamDisconnect(Friends::ClearServer);

		Utils::Hook(0x4CD023, Friends::SetServer, HOOK_JUMP).install()->quick();

		// Show blue icons on the minimap
		Utils::Hook(0x493130, Friends::IsClientInParty, HOOK_JUMP).install()->quick();

		UIScript::Add("LoadFriends", []([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info)
		{
			Friends::UpdateFriends();
		});

		UIScript::Add("JoinFriend", []([[maybe_unused]] const UIScript::Token& token, [[maybe_unused]] const Game::uiInfo_s* info)
		{
			std::lock_guard<std::recursive_mutex> _(Friends::Mutex);
			if (Friends::CurrentFriend >= Friends::FriendsList.size()) return;

			auto& user = Friends::FriendsList[Friends::CurrentFriend];

			if (user.online && user.server.getType() != Game::NA_BAD)
			{
				Party::Connect(user.server);
			}
			else
			{
				Command::Execute("snd_playLocal exit_prestige", false);
			}
		});

		Scheduler::Loop([]
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

			if (stateInterval.elapsed(5s))
			{
				stateInterval.update();

				if (Friends::TriggerUpdate)
				{
					Friends::TriggerUpdate = false;
					Friends::UpdateState();
				}
			}

			if (sortInterval.elapsed(1s))
			{
				sortInterval.update();

				if (Friends::TriggerSort)
				{
					Friends::TriggerSort = false;
					Friends::SortList(true);
				}
			}
		}, Scheduler::Pipeline::CLIENT);

		UIFeeder::Add(61.0f, Friends::GetFriendCount, Friends::GetFriendText, Friends::SelectFriend);

		Scheduler::OnGameShutdown([]
		{
			Friends::ClearPresence("iw4x_server");
			Friends::ClearPresence("iw4x_playing");

#ifdef DEBUG
			if (Steam::Proxy::SteamFriends)
			{
				Steam::Proxy::SteamFriends->ClearRichPresence();
			}
#endif

			if (Steam::Proxy::ClientFriends)
			{
				Steam::Proxy::ClientFriends.invoke<void>("SetPersonaState", Friends::InitialState);
			}
		});

		Scheduler::OnGameInitialized([]
		{
			if (Steam::Proxy::SteamFriends)
			{
				Friends::InitialState = Steam::Proxy::SteamFriends->GetFriendPersonaState(Steam::Proxy::SteamUser_->GetSteamID());
			}

			if (Friends::CLAnonymous.get<bool>() || Friends::IsInvisible() || !Steam::Enabled())
			{
				if (Steam::Proxy::ClientFriends)
				{
					for (const auto id : Friends::GetAppIdList())
					{
						Steam::Proxy::ClientFriends.invoke<void>("ClearRichPresence", id);
					}
				}

				if (Steam::Proxy::SteamFriends)
				{
					Steam::Proxy::SteamFriends->ClearRichPresence();
				}
			}

			Friends::UpdateTimeStamp();
			Friends::UpdateName();
			Friends::UpdateState();

			Friends::UpdateFriends();
		}, Scheduler::Pipeline::CLIENT);
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
