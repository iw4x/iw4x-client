#include "STDInclude.hpp"

namespace Components
{
	std::mutex Bans::AccessMutex;

	bool Bans::IsBanned(Bans::Entry entry)
	{
		auto list = Bans::LoadBans();

		if (entry.first.Bits)
		{
			for (auto& idEntry : list.IDList)
			{
				if (idEntry.Bits == entry.first.Bits)
				{
					return true;
				}
			}
		}

		if (entry.second.full)
		{
			for (auto& ipEntry : list.IPList)
			{
				if (ipEntry.full == entry.second.full)
				{
					return true;
				}
			}
		}

		return false;
	}

	void Bans::InsertBan(Bans::Entry entry)
	{
		auto list = Bans::LoadBans();

		Bans::AccessMutex.lock();

		if (entry.first.Bits)
		{
			bool found = false;
			for (auto& idEntry : list.IDList)
			{
				if (idEntry.Bits == entry.first.Bits)
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				list.IDList.push_back(entry.first);
			}
		}

		if (entry.second.full)
		{
			bool found = false;
			for (auto& ipEntry : list.IPList)
			{
				if (ipEntry.full == entry.second.full)
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				list.IPList.push_back(entry.second);
			}
		}

		// TODO: Write bans

		Bans::AccessMutex.unlock();
	}

	Bans::BanList Bans::LoadBans()
	{
		Bans::BanList list;
		Bans::AccessMutex.lock();

		// TODO: Read bans

		Bans::AccessMutex.unlock();
		return list;
	}

	void Bans::BanClientNum(int num, std::string reason)
	{
		if (!Dvar::Var("sv_running").Get<bool>())
		{
			Logger::Print("Server is not running.\n");
			return;
		}

		if (*Game::svs_numclients <= num)
		{
			Logger::Print("Player %d is not on the server\n", num);
			return;
		}

		Game::client_t* client = &Game::svs_clients[num];

		SteamID guid;
		guid.Bits = client->steamid;

		Bans::InsertBan({ guid, client->addr.ip });

		Game::SV_KickClientError(client, reason);
	}

	Bans::Bans()
	{
		Command::Add("banclient", [] (Command::Params params)
		{
			if (params.Length() < 2) return;

			std::string reason = "EXE_ERR_BANNED_PERM";
			if (params.Length() >= 3) reason = params[2];

			Bans::BanClientNum(atoi(params[1]), reason);
		});
	}

	Bans::~Bans()
	{

	}
}
