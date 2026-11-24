#pragma once

namespace Components
{
  class ServerCommands : public Component
  {
  public:
    ServerCommands();

    using serverCommandHandler = std::function<bool(const Command::Params*)>;
    static void OnCommand(std::int32_t cmd, const serverCommandHandler& callback);

  private:
    static std::unordered_map<std::int32_t, serverCommandHandler> Commands;

    static bool OnServerCommand();

    static bool CG_IsGameServerCommand(char id);

    static void CG_DeployServerCommand_Hk(int localClientNum);
    static void (*CG_DeployServerCommand_Original)(int localClientNum);
  };
}
