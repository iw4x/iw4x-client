#pragma once

namespace Components
{
	class ServerCommands : public Component
	{
	public:
		ServerCommands();

		static void OnCommand(std::int32_t cmd, Utils::Slot<bool(Command::Params*)> cb);

	private:
		static std::unordered_map<std::int32_t, Utils::Slot<bool(Command::Params*)>> Commands;

		static bool OnServerCommand();
		static void OnServerCommandStub();
	};
}
