#pragma once
#include "Game/Structs.hpp"

namespace Components
{
	class Script : public Component
	{
	public:
		Script();
		~Script();

		static int LoadScriptAndLabel(const std::string& script, const std::string& label);

		static void AddFunction(const char* name, Game::xfunction_t func, int type = 0);
		static void AddMethod(const char* name, Game::xmethod_t func, int type = 0);

		static void OnVMShutdown(Utils::Slot<Scheduler::Callback> callback);

		static Game::gentity_t* GetEntity(const Game::scr_entref_t entref);
		static Game::client_t* GetClient(const Game::gentity_t* gentity);

	private:
		static std::string ScriptName;
		static std::vector<int> ScriptHandles;
		static std::unordered_map<std::string, Game::BuiltinFunctionDef> CustomScrFunctions;
		static std::unordered_map<std::string, Game::BuiltinMethodDef> CustomScrMethods;
		static std::vector<std::string> ScriptNameStack;
		static unsigned short FunctionName;
		static std::unordered_map<std::string, std::string> ScriptStorage;
		static std::unordered_map<int, std::string> ScriptBaseProgramNum;
		static std::unordered_map<const char*, const char*> ReplacedFunctions;
		static const char* ReplacedPos;
		static int LastFrameTime;

		static Utils::Signal<Scheduler::Callback> VMShutdownSignal;

		static void CompileError(unsigned int offset, const char* message, ...);
		static void PrintSourcePos(const char* filename, unsigned int offset);

		static void FunctionError();
		static void StoreFunctionNameStub();
		static void RuntimeError(const char* codePos, unsigned int index, const char* msg, const char* dialogMessage);

		static void StoreScriptName(const char* name);
		static void StoreScriptNameStub();

		static void RestoreScriptName();
		static void RestoreScriptNameStub();

		static void LoadGameType();
		static void LoadGameTypeScript();

		static Game::xfunction_t Scr_GetFunctionStub(const char** pName, int* type);
		static Game::xmethod_t Scr_GetMethodStub(const char** pName, int* type);

		static Game::xfunction_t BuiltIn_GetFunctionStub(const char** pName, int* type);
		static Game::xmethod_t Player_GetMethodStub(const char** pName);

		static void ScrShutdownSystemStub(int);
		static void StoreScriptBaseProgramNumStub();
		static void StoreScriptBaseProgramNum();
		static void Scr_PrintPrevCodePosStub();
		static void Scr_PrintPrevCodePos(int);

		static unsigned int SetExpFogStub();

		static const char* GetCodePosForParam(int index);
		static void GetReplacedPos(const char* pos);
		static void SetReplacedPos(const char* what, const char* with);
		static void VMExecuteInternalStub();

		static void AddFunctions();
	};
}
