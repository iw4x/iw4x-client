#pragma once

namespace Components
{
	class ServerCommands : public Component
	{
	public:
		ServerCommands();

		static void OnCommand(std::int32_t cmd, std::function<bool(Command::Params*)> callback);

	private:
		static std::unordered_map<std::int32_t, std::function<bool(Command::Params*)>> Commands;

		static bool OnServerCommand();
		static void CG_DeployServerCommand_Stub();
	};
}
