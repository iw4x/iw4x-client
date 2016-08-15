#include "STDInclude.hpp"

namespace Components
{
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

		// TODO: Write player info into a ban database

		SV_KickClientError(client, reason);
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
