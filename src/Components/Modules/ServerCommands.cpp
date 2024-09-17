#include <STDInclude.hpp>
#include "ServerCommands.hpp"

namespace Components
{
	std::unordered_map<std::int32_t, ServerCommands::serverCommandHandler> ServerCommands::Commands;

	void ServerCommands::OnCommand(std::int32_t cmd, const serverCommandHandler& callback)
	{
		Commands.insert_or_assign(cmd, callback);
	}

	bool ServerCommands::OnServerCommand()
	{
		Command::ClientParams params;
		
		for (const auto& [id, callback] : Commands)
		{
			if (params.size() >= 1)
			{
				if (params.get(0)[0] == id) // Compare ID of server command
				{
					return callback(&params);
				}
			}
		}

		return false;
	}

	__declspec(naked) void ServerCommands::CG_DeployServerCommand_Stub()
	{
		__asm
		{
			push eax
			pushad

			// Missing localClientNum!
			call ServerCommands::OnServerCommand

			mov [esp + 20h], eax
			popad
			pop eax

			test al, al
			jnz jumpback

			test eax, eax
			jle error

			mov eax, dword ptr [edx * 4 + 1AAC634h]
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
		Utils::Hook(0x59449F, CG_DeployServerCommand_Stub).install()->quick();
		Utils::Hook::Nop(0x5944A4, 6);
	}
}
