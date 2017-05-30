#include "STDInclude.hpp"

namespace Components
{
	std::unordered_map < std::int32_t, std::function < bool(Command::Params*) > > ServerCommands::Commands;

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

			push 0x5944AE;
			retn;

		jumpback:
			push 0x594536;
			retn;
		}
	}

	/*void __declspec(naked) OnServerCommandPreFailStub()
	{
		static DWORD jmpAbove = 0x0059449F;
		static DWORD jmpContinue = 0x00593C28;

		__asm mov lastServerCommand, ecx;

		if (lastServerCommand > 0x79)
			__asm jmp jmpAbove;
		else
			__asm jmp jmpContinue;
	}

	void OnServerCommandFailPrint(int type, const char *trash, ...)
	{
		const char *cmd = "";

		for (int i = 1; i < Cmd_Argc(); i++)
			cmd = va("%s %s", cmd, Cmd_Argv(i));

		Com_Printf(type, "Unknown client game command: %i %s\n", lastServerCommand, cmd);
	}*/

	ServerCommands::ServerCommands()
	{
		Utils::Hook(0x59449F, OnServerCommandStub).install()->quick();
	}

	ServerCommands::~ServerCommands()
	{

	}
}
