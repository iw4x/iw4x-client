namespace Utils
{
	namespace Cryptography
	{
		void Initialize();

		class Token
		{
		public:
			Token() { this->TokenString.clear(); };
			Token(const Token& obj) : TokenString(obj.TokenString) { };
			Token(std::string token) : TokenString(token.begin(), token.end()) { };
			Token(std::basic_string<uint8_t> token) : TokenString(token.begin(), token.end()) { };

			Token& operator++ ()
			{
				if (this->TokenString.empty())
				{
					this->TokenString.append(reinterpret_cast<uint8_t*>("\0"), 1);
				}
				else
				{
					for (unsigned int i = (this->TokenString.size() - 1); i >= 0; i--)
					{
						if (this->TokenString[i] == 0xFF)
						{
							this->TokenString[i] = 0;

							if (!i)
							{
								// Prepend here, as /dev/urandom says so ;) https://github.com/IW4x/iw4x-client-node/wikis/technical-information#incrementing-the-token
								this->TokenString = std::basic_string<uint8_t>(reinterpret_cast<uint8_t*>("\0"), 1) + this->TokenString;
								break;
							}
						}
						else
						{
							this->TokenString[i]++;
							break;
						}
					}
				}

				return *this;
			}

			Token operator++ (int)
			{
				Token result = *this;
				++(*this);
				return result;
			}

			bool operator==(const Token& token) const
			{
				return (this->ToString() == token.ToString());
			}

			bool operator!=(const Token& token) const
			{
				return !(*this == token);
			}

			bool operator<(const Token& token) const
			{
				if (*this == token)
				{
					return false;
				}
				else if (this->ToString().size() < token.ToString().size())
				{
					return true;
				}
				else if (this->ToString().size() > token.ToString().size())
				{
					return false;
				}
				else
				{
					auto lStr = this->ToString();
					auto rStr = token.ToString();

					for (unsigned int i = 0; i < lStr.size(); ++i)
					{
						if (lStr[i] < rStr[i])
						{
							return true;
						}
					}
				}

				return false;
			}

			bool operator>(const Token& token) const
			{
				return (token < *this && *this != token);
			}

			bool operator<=(const Token& token) const
			{
				return !(*this > token);
			}

			bool operator>=(const Token& token) const
			{
				return !(*this < token);
			}

			std::string ToString()
			{
				return std::string(this->TokenString.begin(), this->TokenString.end());
			}

			const std::string ToString() const
			{
				return std::string(this->TokenString.begin(), this->TokenString.end());
			}

			std::basic_string<uint8_t> ToUnsignedString()
			{
				return this->TokenString;
			}

			void Clear()
			{
				this->TokenString.clear();
			}

		private:
			std::basic_string<uint8_t> TokenString;
		};

		class Rand
		{
		public:
			static uint32_t GenerateInt();
			static void Initialize();

		private:
			static prng_state State;
		};

		class ECC
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

		class TDES
		{
		public:
			static void Initialize();
			static std::string Encrypt(std::string text, std::string iv, std::string key);
			static std::string Decrpyt(std::string text, std::string iv, std::string key);
		};

		class Tiger
		{
		public:
			static std::string Compute(std::string data, bool hex = false);
			static std::string Compute(const uint8_t* data, size_t length, bool hex = false);
		};

		class SHA256
		{
		public:
			static std::string Compute(std::string data, bool hex = false);
			static std::string Compute(const uint8_t* data, size_t length, bool hex = false);
		};

		class SHA512
		{
		public:
			static std::string Compute(std::string data, bool hex = false);
			static std::string Compute(const uint8_t* data, size_t length, bool hex = false);
		};

		class JenkinsOneAtATime
		{
		public:
			static unsigned int Compute(std::string data);
			static unsigned int Compute(const char *key, size_t len);
		};
	}
}
