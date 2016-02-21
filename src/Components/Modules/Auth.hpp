namespace Components
{
	class Auth : public Component
	{
	public:
		Auth();
		~Auth();
		const char* GetName() { return "Auth"; };
		bool UnitTest();

	private:

		enum AuthState
		{
			STATE_UNKNOWN,
			STATE_NEGOTIATING,
			STATE_VALID,
			STATE_INVALID,
		};

		struct AuthInfo
		{
			Utils::Cryptography::ECDSA::Key publicKey;
			std::string challenge;
			AuthState state;
			int time;
		};

		static AuthInfo ClientAuthInfo[18];

		static void Frame();

		static void RegisterClient(int clientNum);
		static void RegisterClientStub();
	};
}
