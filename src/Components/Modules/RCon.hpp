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
			int timestamp;
			std::string output;
			std::string command;
			std::string challenge;
			Network::Address address;
		};

		class CryptoKey
		{
		public:
			static const Utils::Cryptography::ECC::Key& Get();
		private:
			static bool LoadKey(Utils::Cryptography::ECC::Key& key);
			static Utils::Cryptography::ECC::Key GenerateKey();
			static Utils::Cryptography::ECC::Key LoadOrGenerateKey();
			static Utils::Cryptography::ECC::Key GetKeyInternal();
		};

		static std::unordered_map<std::uint32_t, int> RateLimit;

		static std::vector<std::size_t> RconAddresses;

		static Container RconContainer;
		static Utils::Cryptography::ECC::Key RconKey;

		static std::string Password;

		static Dvar::Var RconPassword;
		static Dvar::Var RconLogRequests;
		static Dvar::Var RconTimeout;

		static void AddCommands();

		static bool IsRateLimitCheckDisabled();
		static bool RateLimitCheck(const Network::Address& address, int time);
		static void RateLimitCleanup(int time);

		static void RconExecuter(const Network::Address& address, std::string data);
	};
}
