namespace Utils
{
	namespace Cryptography
	{
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

			static RSA::Key GenerateKey(int bits);
			static std::string SignMessage(Key key, std::string message);
			static bool VerifyMessage(Key key, std::string message, std::string signature);
		};
	}
}
