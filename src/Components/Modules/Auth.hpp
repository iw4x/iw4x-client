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
		static void IncreaseSecurityLevel(uint32_t level, std::string command = "");

		static uint32_t GetZeroBits(Utils::Cryptography::Token token, std::string publicKey);
		static void IncrementToken(Utils::Cryptography::Token& token, Utils::Cryptography::Token& computeToken, std::string publicKey, uint32_t zeroBits, bool* cancel = nullptr, uint64_t* count = nullptr);

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
			Utils::Cryptography::ECC::Key publicKey;
			std::string challenge;
			AuthState state;
			int time;
		};

		struct TokenIncrementing
		{
			bool cancel;
			bool generating;
			std::thread thread;
			uint32_t targetLevel;
			int startTime;
			std::string command;
			uint64_t hashes;
		};

		static AuthInfo ClientAuthInfo[18];
		static TokenIncrementing TokenContainer;

		static Utils::Cryptography::Token GuidToken;
		static Utils::Cryptography::Token ComputeToken;
		static Utils::Cryptography::ECC::Key GuidKey;

		static void Frame();

		static void RegisterClient(int clientNum);
		static void RegisterClientStub();
	};
}
