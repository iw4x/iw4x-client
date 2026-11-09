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

  ServerCommands::ServerCommands()
  {
    // Server command receive hook
    Utils::Hook(0x59449F, ServerCommands_CG_DeployServerCommand_Stub).install()->quick();
    Utils::Hook::Nop(0x5944A4, 6);
  }
}

bool ServerCommands_OnServerCommand()
{
  return Components::ServerCommands::OnServerCommand();
}
