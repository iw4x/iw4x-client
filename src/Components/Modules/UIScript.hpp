namespace Components
{
	class UIScript : public Component
	{
	public:
		UIScript();
		~UIScript();

#ifdef DEBUG
		const char* GetName() { return "UIScript"; };
#endif

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

		typedef void(Callback)(Token token);
		typedef void(CallbackRaw)();

		static void Add(std::string name, Callback* callback);
		static void Add(std::string name, CallbackRaw* callback);

		static void AddOwnerDraw(int ownerdraw, CallbackRaw* callback);

	private:
		static void OwnerDrawHandleKeyStub(int ownerDraw, int flags, float *special, int key);
		static bool RunMenuScript(const char* name, const char** args);
		static void RunMenuScriptStub();

		static std::map<std::string, wink::slot<Callback>> UIScripts;
		static std::map<int, wink::slot<CallbackRaw>> UIOwnerDraws;
	};
}
