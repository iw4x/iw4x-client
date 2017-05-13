#pragma once
#include "Game/Structs.hpp"

namespace Components
{
	class Script : public Component
	{
	public:
		class Function
		{
		public:
			Function(std::string _name, Game::scr_function_t _callback, bool _dev) : name(_name), callback(_callback), dev(_dev) {}

			const char* getName() const { return this->name.data(); }
			bool isDev() const { return this->dev; }
			Game::scr_function_t getFunction() const { return this->callback; }

		private:
			std::string name;
			Game::scr_function_t callback;
			bool dev;
		};

		Script();
		~Script();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* getName() override { return "Script"; };
#endif

		static int LoadScriptAndLabel(std::string script, std::string label);
		static void AddFunction(std::string name, Game::scr_function_t function, bool isDev = false);

	private:
		static std::string ScriptName;
		static std::vector<int> ScriptHandles;
		static std::vector<Function> ScriptFunctions;
		static std::vector<std::string> ScriptNameStack;
		static unsigned short FunctionName;

		static void CompileError(unsigned int offset, const char* message, ...);
		static void PrintSourcePos(const char* filename, unsigned int offset);

		static void FunctionError();
		static void StoreFunctionNameStub();

		static void StoreScriptName(const char* name);
		static void StoreScriptNameStub();

		static void RestoreScriptName();
		static void RestoreScriptNameStub();

		static void LoadGameType();
		static void LoadGameTypeScript();

		static Game::scr_function_t GetFunction(const char** name, int* isDev);
		static void GetFunctionStub();

		static int SetExpFogStub();
	};
}
