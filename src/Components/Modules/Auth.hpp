namespace Components
{
	class Auth : public Component
	{
	public:
		Auth();
		~Auth();
		const char* GetName() { return "Auth"; };
		bool UnitTest();

		static void StoreKey();
		static void LoadKey(bool force = false);
		static unsigned int GetKeyHash();

		static uint32_t GetSecurityLevel();
		static uint32_t GetZeroBits(Utils::Cryptography::Token token, std::string publicKey);
		static void IncrementToken(Utils::Cryptography::Token& token, std::string publicKey, uint32_t zeroBits);

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

		static Utils::Cryptography::Token GuidToken;
		static Utils::Cryptography::ECDSA::Key GuidKey;

		static void Frame();

		static void RegisterClient(int clientNum);
		static void RegisterClientStub();
	};
}
