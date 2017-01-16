namespace Components
{
	class UIScript : public Component
	{
	public:
		UIScript();
		~UIScript();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* getName() { return "UIScript"; };
#endif

		class Token
		{
		public:
			Token() : token(0) {};
			Token(const char** args) : token(0) { this->parse(args); };
			Token(const Token &obj) { this->token = obj.token; };

			template<typename T> T get();
			bool isValid();


		private:
			char* token;

			void parse(const char** args);
		};

		typedef void(Callback)(Token token);
		typedef void(CallbackRaw)();

		static void Add(std::string name, Utils::Slot<Callback> callback);
		static void AddOwnerDraw(int ownerdraw, Utils::Slot<CallbackRaw> callback);

	private:
		static void OwnerDrawHandleKeyStub(int ownerDraw, int flags, float *special, int key);
		static bool RunMenuScript(const char* name, const char** args);
		static void RunMenuScriptStub();

		static std::unordered_map<std::string, Utils::Slot<Callback>> UIScripts;
		static std::unordered_map<int, Utils::Slot<CallbackRaw>> UIOwnerDraws;
	};
}
