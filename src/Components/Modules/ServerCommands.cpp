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
    // Look the command up the way CG_DeployServerCommand() does: an id past
    // 'z' isn't one of the game's commands, and neither is one whose case in
    // the switch is the one that does nothing.
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
    // On x86 we go in where the game finds the command unknown, which on x64
    // is spread over its switch. So instead we go in before it and look the
    // command up ourselves (see CG_IsGameServerCommand()).
    //
    // Note also that x64 numbers the game's commands differently from x86, so
    // an id that is free on x86 may be one of the game's own on x64 (M, for
    // one), and then the game handles it before we get to see it. That's
    // presumably the cause if one of our commands seems to do nothing on x64
    // only (see Compatibility for how we deal with the ones we know of).
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
