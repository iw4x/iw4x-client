namespace Utils
{
	namespace Cryptography
	{
		class Rand
		{
		public:
			static uint32_t GenerateInt();
		};

		class ECDSA
		{
		public:
			class Key
			{
			public:
				Key() { ZeroMemory(&this->KeyStorage, sizeof(this->KeyStorage)); };
				Key(ecc_key* key) : Key(*key) {};
				Key(ecc_key key) : KeyStorage(key) {};
				Key(const Key& obj) : KeyStorage(obj.KeyStorage) {};

				~Key() {}

				ecc_key* GetKeyPtr()
				{
					return &this->KeyStorage;
				}

				std::string GetPublicKey()
				{
					uint8_t buffer[0x1000] = { 0 };
					DWORD length = sizeof(buffer);

					if (ecc_ansi_x963_export(this->GetKeyPtr(), buffer, &length) == CRYPT_OK)
					{
						return std::string(reinterpret_cast<char*>(buffer), length);
					}

					return "";
				}

			private:
				ecc_key KeyStorage;
			};

			static Key GenerateKey(int bits);
			static std::string SignMessage(Key key, std::string message);
			static bool VerifyMessage(Key key, std::string message, std::string signature);
		};

		class RSA
		{
		public:
			class Key
			{
			public:
				Key() { ZeroMemory(&this->KeyStorage, sizeof(this->KeyStorage)); };
				Key(rsa_key* key) : Key(*key) {};
				Key(rsa_key key) : KeyStorage(key) {};
				Key(const Key& obj) : KeyStorage(obj.KeyStorage) {};

				~Key() {}

				rsa_key* GetKeyPtr()
				{
					return &this->KeyStorage;
				}

			private:
				rsa_key KeyStorage;
			};

			static Key GenerateKey(int bits);
			static std::string SignMessage(Key key, std::string message);
			static bool VerifyMessage(Key key, std::string message, std::string signature);
		};
	}
}
