#pragma once

namespace Components
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

		static Game::client_t* GetClient(const Game::gentity_t* gentity);

		static const char* GetCodePosForParam(int index);

		// Probably a macro 'originally' but this is fine
		static Game::gentity_s* Scr_GetPlayerEntity(Game::scr_entref_t entref)
		{
			if (entref.classnum != 0)
			{
				Game::Scr_ObjectError("not an entity");
				return nullptr;
			}

			assert(entref.entnum < Game::MAX_GENTITIES);

			auto* ent = &Game::g_entities[entref.entnum];
			if (ent->client == nullptr)
			{
				Game::Scr_ObjectError(Utils::String::VA("entity %i is not a player", entref.entnum));
				return nullptr;
			}

			return ent;
		}

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

		static int LastFrameTime;

		static std::unordered_map<std::string, int> ScriptMainHandles;
		static std::unordered_map<std::string, int> ScriptInitHandles;

		static std::unordered_map<const char*, const char*> ReplacedFunctions;
		static const char* ReplacedPos;

		static void Scr_LoadGameType_Stub();
		static void Scr_StartupGameType_Stub();
		static void GScr_LoadGameTypeScript_Stub();

		static Game::BuiltinFunction BuiltIn_GetFunctionStub(const char** pName, int* type);
		static Game::BuiltinMethod BuiltIn_GetMethodStub(const char** pName, int* type);

		static unsigned int SetExpFogStub();

		static void GetReplacedPos(const char* pos);
		static void SetReplacedPos(const char* what, const char* with);
		static void VMExecuteInternalStub();

		static void AddFunctions();
	};
}
