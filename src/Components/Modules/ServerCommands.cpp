#include "STDInclude.hpp"

namespace Components
{
	std::unordered_map < std::int32_t, std::function < bool(Command::Params*) > > ServerCommands::Commands;
	std::uint32_t ServerCommands::lastServerCommand;

	void ServerCommands::OnCommand(std::int32_t cmd, std::function < bool(Command::Params*) > cb)
	{
		Commands[cmd] = cb;
	}

	bool ServerCommands::OnServerCommand()
	{
		Command::ClientParams params(*Game::cmd_id);
		
		for (auto &ServerCommandCB : Commands)
		{
			if (params.length() >= 1)
			{
				if (params.get(0)[0] == ServerCommandCB.first)
				{
					return ServerCommandCB.second(&params);
				}
			}
		}

		return false;
	}

	void __declspec(naked) ServerCommands::OnServerCommandStub()
	{
		__asm
		{
			call OnServerCommand;
			test al, al;
			jnz jumpback;

			push 5944AEh;
			retn;

		jumpback:
			push 594536h;
			retn;
		}
	}

	void __declspec(naked) ServerCommands::OnServerCommandPreFailStub()
	{
		__asm
		{
			mov lastServerCommand, ecx;
			cmp ecx, 79h;

			jl above;

			push 59449Fh;
			retn;

		above:
			push 593C28h;
			retn;
		}
	}

	void ServerCommands::OnServerCommandFailPrint(int type, const char *, ...)
	{
		Command::ClientParams params(*Game::cmd_id);
		const char *cmd = "";

		for (std::size_t i = 1; i < params.length(); i++)
			cmd = Utils::String::VA("%s %s", cmd, params.get(i));

		Game::Com_Printf(type, "Unknown client game command: %i %s\n", lastServerCommand, cmd);
	}

	void __declspec(naked) ServerCommands::OnServerCommandFailPrintStub()
	{
		__asm
		{
			pushad;
			call OnServerCommandFailPrint;
			popad;

			push 5944C0h;
			retn;
		}
	}

	ServerCommands::ServerCommands()
	{
		// Server command receive hook
		Utils::Hook(0x59449F, OnServerCommandStub).install()->quick();

		// Server command fail hooks
		Utils::Hook(0x593C1F, OnServerCommandPreFailStub).install()->quick();
		Utils::Hook(0x5944BB, OnServerCommandFailPrintStub).install()->quick();
		Utils::Hook::Set<std::uint8_t>(0x5944D3, 0xEB);
	}

	ServerCommands::~ServerCommands()
	{
		Commands.clear();
	}
}
