#pragma once

namespace Components
{
	class Script : public Component
	{
	public:
		Script();

		static void AddFunction(const std::string& name, Game::BuiltinFunction func, bool type = false);
		static void AddMethod(const std::string& name, Game::BuiltinMethod func, bool type = false);

		static Game::client_t* GetClient(const Game::gentity_t* gentity);

		static const char* GetCodePosForParam(int index);

	private:
		struct ScriptFunction
		{
			Game::BuiltinFunction actionFunc;
			bool type;
		};

		struct ScriptMethod
		{
			Game::BuiltinMethod actionFunc;
			bool type;
		};

		static std::unordered_map<std::string, ScriptFunction> CustomScrFunctions;
		static std::unordered_map<std::string, ScriptMethod> CustomScrMethods;

		static std::string ScriptName;
		static std::vector<std::string> ScriptNameStack;
		static unsigned short FunctionName;
		static std::unordered_map<int, std::string> ScriptBaseProgramNum;
		static int LastFrameTime;

		static std::vector<int> ScriptMainHandles;
		static std::vector<int> ScriptInitHandles;

		static std::unordered_map<const char*, const char*> ReplacedFunctions;
		static const char* ReplacedPos;

		static void CompileError(unsigned int offset, const char* message, ...);
		static void PrintSourcePos(const char* filename, unsigned int offset);

		static void FunctionError();
		static void StoreFunctionNameStub();
		static void RuntimeError(const char* codePos, unsigned int index, const char* msg, const char* dialogMessage);

		static void StoreScriptName(const char* name);
		static void StoreScriptNameStub();

		static void RestoreScriptName();
		static void RestoreScriptNameStub();

		static void Scr_LoadGameType_Stub();
		static void Scr_StartupGameType_Stub();
		static void GScr_LoadGameTypeScript_Stub();

		static bool IsDeprecated(const std::string& name);
		static Game::BuiltinFunction BuiltIn_GetFunctionStub(const char** pName, int* type);
		static Game::BuiltinMethod BuiltIn_GetMethodStub(const char** pName, int* type);

		static void StoreScriptBaseProgramNumStub();
		static void StoreScriptBaseProgramNum();
		static void Scr_PrintPrevCodePosStub();
		static void Scr_PrintPrevCodePos(int);

		static unsigned int SetExpFogStub();

		static void GetReplacedPos(const char* pos);
		static void SetReplacedPos(const char* what, const char* with);
		static void VMExecuteInternalStub();

		static void AddFunctions();
	};
}
