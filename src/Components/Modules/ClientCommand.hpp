#pragma once

namespace Components
{
	class ClientCommand : public Component
	{
	public:
		typedef void(Callback)(Game::gentity_s* entity);

		ClientCommand();
		~ClientCommand();
		static void Add(const char* name, Utils::Slot<Callback> callback);
		static bool CheatsOk(const Game::gentity_s* ent);

	private:
		static std::unordered_map<std::string, Utils::Slot<Callback>> FunctionMap;

		static bool CallbackHandler(Game::gentity_s* ent, const char* cmd);
		static void ClientCommandStub(const int clientNum);
		static void AddCheatCommands();
		static void AddScriptFunctions();
	};
}
