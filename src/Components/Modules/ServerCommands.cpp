#include "STDInclude.hpp"

namespace Components
{
	std::unordered_map<std::int32_t, std::function<bool(Command::Params*)>> ServerCommands::Commands;
	std::uint32_t ServerCommands::LastServerCommand;

	void ServerCommands::OnCommand(std::int32_t cmd, std::function<bool(Command::Params*)> cb)
	{
		ServerCommands::Commands[cmd] = cb;
	}

	bool ServerCommands::OnServerCommand()
	{
		Command::ClientParams params(*Game::cmd_id);
		
		for (auto &serverCommandCB : ServerCommands::Commands)
		{
			if (params.length() >= 1)
			{
				if (params.get(0)[0] == serverCommandCB.first)
				{
					return serverCommandCB.second(&params);
				}
			}
		}

		return false;
	}

	__declspec(naked) void ServerCommands::OnServerCommandStub()
	{
		__asm
		{
			push eax
			pushad
			call ServerCommands::OnServerCommand
			mov [esp + 20h], eax
			popad
			pop eax

			test al, al
			jnz jumpback

			push 5944AEh
			retn

		jumpback:
			push 594536h
			retn
		}
	}

	__declspec(naked) void ServerCommands::OnServerCommandPreFailStub()
	{
		__asm
		{
			mov ServerCommands::LastServerCommand, ecx
			cmp ecx, 79h

			jl above

			push 59449Fh
			retn

		above:
			push 593C28h
			retn
		}
	}

	void ServerCommands::OnServerCommandFailPrint(int type, const char *, ...)
	{
		Command::ClientParams params(*Game::cmd_id);
		Game::Com_Printf(type, "Unknown client game command: %i %s\n", LastServerCommand, params.join(1));
	}

	__declspec(naked) void ServerCommands::OnServerCommandFailPrintStub()
	{
		__asm
		{
			pushad
			call ServerCommands::OnServerCommandFailPrint
			popad

			push 5944C0h
			retn
		}
	}

	ServerCommands::ServerCommands()
	{
		// Server command receive hook
		Utils::Hook(0x59449F, ServerCommands::OnServerCommandStub).install()->quick();

		// Server command fail hooks
		Utils::Hook(0x593C1F, ServerCommands::OnServerCommandPreFailStub).install()->quick();
		Utils::Hook(0x5944BB, ServerCommands::OnServerCommandFailPrintStub).install()->quick();
		Utils::Hook::Set<std::uint8_t>(0x5944D3, 0xEB);
	}

	ServerCommands::~ServerCommands()
	{
		ServerCommands::Commands.clear();
	}
}
