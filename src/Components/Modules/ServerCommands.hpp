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
		static void CG_DeployServerCommand_Stub();
	};
}
