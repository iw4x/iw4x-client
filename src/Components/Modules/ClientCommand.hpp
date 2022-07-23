#pragma once

namespace Components
{
	class ClientCommand : public Component
	{
	public:
		ClientCommand();

		static void Add(const char* name, const std::function<void(Game::gentity_s*, Command::ServerParams*)>& callback);
		static bool CheatsOk(const Game::gentity_s* ent);

	private:
		static std::unordered_map<std::string, std::function<void(Game::gentity_s*, Command::ServerParams*)>> HandlersSV;

		static void ClientCommandStub(int clientNum);
		static void AddCheatCommands();
		static void AddDevelopmentCommands();
		static void AddScriptFunctions();
	};
}
