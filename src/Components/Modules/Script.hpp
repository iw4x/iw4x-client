namespace Components
{
	class Script : public Component
	{
	public:
		Script();
		~Script();
		const char* GetName() { return "Script"; };

		static int LoadScriptAndLabel(std::string script, std::string label);

	private:
		static std::string ScriptName;
		static std::vector<int> ScriptHandles;
		static std::vector<std::string> ScriptNameStack;
		static unsigned short FunctionName;

		static void CompileError(int offset, const char* message, ...);
		static void PrintSourcePos(const char* filename, int offset);

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
