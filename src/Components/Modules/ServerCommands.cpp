#include "ServerCommands.hpp"

namespace Components
{
  std::unordered_map<std::int32_t, ServerCommands::serverCommandHandler> ServerCommands::Commands;
  void (*ServerCommands::CG_DeployServerCommand_Original)(int localClientNum);

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

  bool ServerCommands::CG_IsGameServerCommand(char id)
  {
    // Look the command up the way CG_DeployServerCommand() does: any id past
    // 'z' and any whose case in its switch is the one that does nothing is
    // not a command of the game.
    //
    constexpr std::uintptr_t image(0x140000000);
    constexpr std::uintptr_t unknownCase(0x1400E7567);

    const auto* caseIndices(reinterpret_cast<const std::uint8_t*>(0x1400E7650));
    const auto* caseOffsets(reinterpret_cast<const std::uint32_t*>(0x1400E759C));

    const auto i(static_cast<unsigned char>(id));

    return i <= 'z' && image + caseOffsets[caseIndices[i]] != unknownCase;
  }

  void ServerCommands::CG_DeployServerCommand_Hk(int localClientNum)
  {
    // Note that on x86 we go in where the game finds the command unknown,
    // which on x64 is spread over its switch. So we go in before it and look
    // the command up ourselves.
    //
    // Note also that x64 numbers the game's commands differently from x86,
    // so that ids which are free on x86 may be the game's own on x64 (such as
    // 'M'), and the game handles them before us.
    //
    Command::ClientParams params;

    if (params.size() < 1 || CG_IsGameServerCommand(params.get(0)[0]) || !OnServerCommand())
    {
      CG_DeployServerCommand_Original(localClientNum);
    }
  }

  ServerCommands::ServerCommands()
  {
    // Server command receive hook
    CG_DeployServerCommand_Original = Utils::Hook::Detour(0x1400E64C0, 6, &CG_DeployServerCommand_Hk);
  }
}
