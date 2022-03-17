#include <STDInclude.hpp>

namespace Components
{
	std::unordered_map<std::int32_t, Utils::Slot<bool(Command::Params*)>> ServerCommands::Commands;

	void ServerCommands::OnCommand(std::int32_t cmd, Utils::Slot<bool(Command::Params*)> cb)
	{
		ServerCommands::Commands[cmd] = cb;
	}

	bool ServerCommands::OnServerCommand()
	{
		Command::ClientParams params;
		
		for (const auto& serverCommandCB : ServerCommands::Commands)
		{
			if (params.size() >= 1)
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

			test eax, eax
			jle error

			mov eax, DWORD PTR[edx * 4 + 1AAC634h]
			mov eax, [eax]

			push 5944B3h
			retn

		error:
			push 5944AEh
			retn

		jumpback:
			push 594536h
			retn
		}
	}

	ServerCommands::ServerCommands()
	{
		// Server command receive hook
		Utils::Hook(0x59449F, ServerCommands::OnServerCommandStub).install()->quick();
	}

	ServerCommands::~ServerCommands()
	{
		ServerCommands::Commands.clear();
	}
}
