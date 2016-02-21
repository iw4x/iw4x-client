namespace Utils
{
	namespace Cryptography
	{
		class Rand
		{
		public:
			static uint32_t GenerateInt();
			static void Initialize();

		private:
			static prng_state State;
		};

		class ECDSA
		{
		public:
			class Key
			{
			public:
				Key() : KeyStorage(new ecc_key)
				{
					ZeroMemory(this->KeyStorage.get(), sizeof(*this->GetKeyPtr()));
				};
				Key(ecc_key* key) : Key() { if(key) std::memmove(this->GetKeyPtr(), key, sizeof(*key)); };
				Key(ecc_key key) : Key(&key) {};
				~Key() 
				{
					if (this->KeyStorage.use_count() <= 1)
					{
						this->Free();
					}
				};

				bool IsValid()
				{
					return (!Utils::MemIsSet(this->GetKeyPtr(), 0, sizeof(*this->GetKeyPtr())));
				}

				ecc_key* GetKeyPtr()
				{
					return this->KeyStorage.get();
				}

				std::string GetPublicKey()
				{
					uint8_t buffer[512] = { 0 };
					DWORD length = sizeof(buffer);

					if (ecc_ansi_x963_export(this->GetKeyPtr(), buffer, &length) == CRYPT_OK)
					{
						return std::string(reinterpret_cast<char*>(buffer), length);
					}

					return "";
				}

				void Set(std::string pubKeyBuffer)
				{
					this->Free();

					if (ecc_ansi_x963_import(reinterpret_cast<const uint8_t*>(pubKeyBuffer.data()), pubKeyBuffer.size(), this->GetKeyPtr()) != CRYPT_OK)
					{
						ZeroMemory(this->KeyStorage.get(), sizeof(*this->GetKeyPtr()));
					}
				}

				void Import(std::string key, int type = PK_PRIVATE)
				{
					this->Free();

					if (ecc_import(reinterpret_cast<const uint8_t*>(key.data()), key.size(), this->GetKeyPtr()) != CRYPT_OK)
					{
						ZeroMemory(this->KeyStorage.get(), sizeof(*this->GetKeyPtr()));
					}
				}

				std::string Export(int type = PK_PRIVATE)
				{
					uint8_t buffer[4096] = { 0 };
					DWORD length = sizeof(buffer);

					if (ecc_export(buffer, &length, type, this->GetKeyPtr()) == CRYPT_OK)
					{
						return std::string(reinterpret_cast<char*>(buffer), length);
					}

					return "";
				}

				void Free()
				{
					if (this->IsValid())
					{
						ecc_free(this->GetKeyPtr());
					}

					ZeroMemory(this->GetKeyPtr(), sizeof(*this->GetKeyPtr()));
				}

			private:
				std::shared_ptr<ecc_key> KeyStorage;
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
				Key() : KeyStorage(new rsa_key)
				{
					ZeroMemory(this->KeyStorage.get(), sizeof(*this->GetKeyPtr()));
				};
				Key(rsa_key* key) : Key() { if (key) std::memmove(this->GetKeyPtr(), key, sizeof(*key)); };
				Key(rsa_key key) : Key(&key) {};
				~Key()
				{
					if (this->KeyStorage.use_count() <= 1)
					{
						this->Free();
					}
				};

				rsa_key* GetKeyPtr()
				{
					return this->KeyStorage.get();
				}

				bool IsValid()
				{
					return (!Utils::MemIsSet(this->GetKeyPtr(), 0, sizeof(*this->GetKeyPtr())));
				}

				void Free()
				{
					if (this->IsValid())
					{
						rsa_free(this->GetKeyPtr());
					}

					ZeroMemory(this->GetKeyPtr(), sizeof(*this->GetKeyPtr()));
				}

			private:
				std::shared_ptr<rsa_key> KeyStorage;
			};

			static Key GenerateKey(int bits);
			static std::string SignMessage(Key key, std::string message);
			static bool VerifyMessage(Key key, std::string message, std::string signature);
		};
	}
}
