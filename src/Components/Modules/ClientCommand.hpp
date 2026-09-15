#pragma once

namespace Components
{
	class ClientCommand : public Component
	{
	public:
		ClientCommand();

		static void Add(const char* name, const std::function<void(Game::gentity_s*, const Command::ServerParams*)>& callback);
		static bool CheatsOk(const Game::gentity_s* ent);

	private:
		static std::unordered_map<std::string, std::function<void(Game::gentity_s*, const Command::ServerParams*)>> HandlersSV;

		static bool CheatsEnabled;

		class CheatsScopedLock
		{
		public:
			CheatsScopedLock();
			~CheatsScopedLock();
		};

		static void ClientCommandStub(int clientNum);
		static void AddCheatCommands();
		static void AddDevelopmentCommands();
		static void AddServerCommands();

		static Game::gentity_s* GetPlayerEntity(const char* input);
		static void Give(Game::gentity_s* ent, const char* weaponName);
		static void Take(Game::gentity_s* ent, const char* weaponName);

		static void AddScriptFunctions();
		static void AddScriptMethods();

		static const char* EntInfoLine(int entNum);
		static void G_DumpEntityDebugInfoToConsole(bool logfileOnly);
		static void G_DumpEntityDebugInfoToCSV(const char* filenameSuffix);

		static void Cmd_Noclip_f(Game::gentity_s* ent, const Command::ServerParams* params);
		static void Cmd_UFO_f(Game::gentity_s* ent, const Command::ServerParams* params);

		static void GiveMaxAmmo(Game::gentity_s* ent);
		static void GiveAllWeapons(Game::gentity_s* ent);
		static void SetOffhandClass(Game::gentity_s* ent, unsigned int weaponIndex);
	};
}
