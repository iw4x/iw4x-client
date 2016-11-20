namespace Components
{
	class Auth : public Component
	{
	public:
		Auth();
		~Auth();

#if defined(DEBUG) || defined(FORCE_UNIT_TESTS)
		const char* getName() { return "Auth"; };
#endif

		bool unitTest();

		static void StoreKey();
		static void LoadKey(bool force = false);
		static unsigned int GetKeyHash();

		static uint32_t GetSecurityLevel();
		static void IncreaseSecurityLevel(uint32_t level, std::string command = "");

		static uint32_t GetZeroBits(Utils::Cryptography::Token token, std::string publicKey);
		static void IncrementToken(Utils::Cryptography::Token& token, Utils::Cryptography::Token& computeToken, std::string publicKey, uint32_t zeroBits, bool* cancel = nullptr, uint64_t* count = nullptr);

	private:

		class TokenIncrementing
		{
		public:
			bool cancel;
			bool generating;
			std::thread thread;
			uint32_t targetLevel;
			int startTime;
			std::string command;
			uint64_t hashes;
		};

		static TokenIncrementing TokenContainer;

		static Utils::Cryptography::Token GuidToken;
		static Utils::Cryptography::Token ComputeToken;
		static Utils::Cryptography::ECC::Key GuidKey;

		static void SendConnectDataStub(Game::netsrc_t sock, Game::netadr_t adr, const char *format, int len);
		static void ParseConnectData(Game::msg_t* msg, Game::netadr_t addr);
		static void DirectConnectStub();

		static void Frame();
	};
}
