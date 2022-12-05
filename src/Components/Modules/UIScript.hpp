#pragma once

namespace Components
{
	class UIScript : public Component
	{
	public:
		UIScript();
		~UIScript();

		class Token
		{
		public:
			Token() : token(nullptr) {}
			Token(const char** args) : token(nullptr) { this->parse(args); }
			Token(const Token &obj) { this->token = obj.token; }

			template<typename T> T get() const;
			bool isValid() const;

		private:
			char* token;

			void parse(const char** args);
		};

		using UIScriptHandler = std::function<void(const Token& token, const Game::uiInfo_s* info)>;

		static Game::uiInfo_s* UI_GetClientInfo(int localClientNum);

		static void Add(const std::string& name, const UIScriptHandler& callback);
		static void AddOwnerDraw(int ownerdraw, const std::function<void()>& callback);

	private:
		static void OwnerDrawHandleKeyStub(int ownerDraw, int flags, float *special, int key);
		static bool RunMenuScript(const char* name, const char** args);
		static void RunMenuScriptStub();

		static std::unordered_map<std::string, UIScriptHandler> UIScripts;
		static std::unordered_map<int, std::function<void()>> UIOwnerDraws;
	};
}
