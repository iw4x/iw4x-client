namespace Components
{
	class Script : public Component
	{
	public:
		Script();
		~Script();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* getName() { return "Script"; };
#endif

		static int LoadScriptAndLabel(std::string script, std::string label);

	private:
		static std::string ScriptName;
		static std::vector<int> ScriptHandles;
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
	};
}
