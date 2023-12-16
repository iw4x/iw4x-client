#pragma once

namespace Components
{
	class RCon : public Component
	{
	public:
		RCon();

	private:
		class Container
		{
		public:
			int timestamp{};
			std::string output{};
			std::string command{};
			std::string challenge{};
			Network::Address address{};
		};

		class CryptoKeyECC
		{
		public:
			static Utils::Cryptography::ECC::Key& Get();
		private:
			static bool LoadKey(Utils::Cryptography::ECC::Key& key);
			static Utils::Cryptography::ECC::Key GenerateKey();
			static Utils::Cryptography::ECC::Key LoadOrGenerateKey();
			static Utils::Cryptography::ECC::Key GetKeyInternal();
		};

		class CryptoKeyRSA
		{
		public:
			static bool HasPublicKey();

			static Utils::Cryptography::RSA::Key& GetPublicKey();
			static Utils::Cryptography::RSA::Key& GetPrivateKey();

		private:
			static Utils::Cryptography::RSA::Key GenerateKeyPair();

			static Utils::Cryptography::RSA::Key LoadPublicKey();
			static Utils::Cryptography::RSA::Key GetPublicKeyInternal();

			static bool LoadPrivateKey(Utils::Cryptography::RSA::Key& key);
			static Utils::Cryptography::RSA::Key LoadOrGeneratePrivateKey();
			static Utils::Cryptography::RSA::Key GetPrivateKeyInternal();
		};

		static std::unordered_map<std::uint32_t, int> RateLimit;

		static std::vector<std::size_t> RConAddresses;

		static Container RConContainer;
		static Utils::Cryptography::ECC::Key RConKey;

		static std::string Password;

		static std::string RConOutputBuffer;

		static Dvar::Var RConPassword;
		static Dvar::Var RConLogRequests;
		static Dvar::Var RConTimeout;

		static void AddCommands();

		static bool IsRateLimitCheckDisabled();
		static bool RateLimitCheck(const Network::Address& address, int time);
		static void RateLimitCleanup(int time);

		static void RConExecutor(const Network::Address& address, std::string data);
		static void RConSafeExecutor(const Network::Address& address, std::string command);
	};
}
