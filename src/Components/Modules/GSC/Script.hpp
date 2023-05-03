#pragma once

namespace Components::GSC
{
	class Script : public Component
	{
	public:
		Script();

		using scriptNames = std::vector<std::string>;
		static void AddFunction(const std::string& name, Game::BuiltinFunction func, bool type = false);
		static void AddMethod(const std::string& name, Game::BuiltinMethod func, bool type = false);

		static void AddFuncMultiple(Game::BuiltinFunction func, bool type, scriptNames);
		static void AddMethMultiple(Game::BuiltinMethod func, bool type, scriptNames);

		static Game::client_s* GetClient(const Game::gentity_s* gentity);
		// Probably a macro 'originally' but this is fine
		static Game::gentity_s* Scr_GetPlayerEntity(Game::scr_entref_t entref);

	private:
		struct ScriptFunction
		{
			Game::BuiltinFunction actionFunc;
			bool type;
			scriptNames aliases;
		};

		struct ScriptMethod
		{
			Game::BuiltinMethod actionFunc;
			bool type;
			scriptNames aliases;
		};

		static std::vector<ScriptFunction> CustomScrFunctions;
		static std::vector<ScriptMethod> CustomScrMethods;

		static std::unordered_map<std::string, int> ScriptMainHandles;
		static std::unordered_map<std::string, int> ScriptInitHandles;

		static void LoadCustomScriptsFromFolder(const char* dir);
		static void LoadCustomScripts();

		static void Scr_LoadGameType_Stub();
		static void Scr_StartupGameType_Stub();
		static void GScr_LoadGameTypeScript_Stub();

		static Game::BuiltinFunction BuiltIn_GetFunctionStub(const char** pName, int* type);
		static Game::BuiltinMethod BuiltIn_GetMethodStub(const char** pName, int* type);

		static unsigned int SetExpFogStub();
	};
}
