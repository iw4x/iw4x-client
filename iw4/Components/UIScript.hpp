namespace Components
{
	class UIScript : public Component
	{
	public:
		UIScript();
		~UIScript();
		const char* GetName() { return "UIScript"; };

		class Token
		{
		public:
			Token(const char** args) : token(0) { this->Parse(args); };
			Token(const Token &obj) { this->token = obj.token; };

			template<typename T> T Get();
			bool IsValid();


		private:
			char* token;

			void Parse(const char** args);
		};

		typedef void(*Callback)(Token token);
		typedef void(*CallbackRaw)();

		static void Add(std::string name, Callback callback);
		static void Add(std::string name, CallbackRaw callback);

	private:

		static bool RunMenuScript(const char* name, const char** args);
		static void RunMenuScriptStub();

		static std::map<std::string, Callback> UIScripts;
	};
}
