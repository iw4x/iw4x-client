#pragma once

extern "C"
{
  void ServerCommands_CG_DeployServerCommand_Stub();

  bool ServerCommands_OnServerCommand();
}

namespace Components
{
  class ServerCommands : public Component
  {
  public:
    ServerCommands();

    using serverCommandHandler = std::function<bool(const Command::Params*)>;
    static void OnCommand(std::int32_t cmd, const serverCommandHandler& callback);

  private:
    friend bool (::ServerCommands_OnServerCommand)();

    static std::unordered_map<std::int32_t, serverCommandHandler> Commands;

    static bool OnServerCommand();
  };
}
