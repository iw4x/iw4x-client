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

		static Container RconContainer;
		static Utils::Cryptography::ECC::Key RconKey;

		static std::string Password;

		static Dvar::Var RconPassword;
		static Dvar::Var RconLogRequests;

		static void AddCommands();
	};
}
