#pragma once

namespace Utils
{
	namespace Cryptography
	{
		void Initialize();

		class Token
		{
		public:
			Token() { this->tokenString.clear(); };
			Token(const Token& obj) : tokenString(obj.tokenString) { };
			Token(const std::string& token) : tokenString(token.begin(), token.end()) { };
			Token(const std::basic_string<uint8_t>& token) : tokenString(token.begin(), token.end()) { };

			Token& operator++ ()
			{
				if (this->tokenString.empty())
				{
					this->tokenString.append(reinterpret_cast<uint8_t*>(const_cast<char *>("\0")), 1);
				}
				else
				{
					for (int i = static_cast<int>(this->tokenString.size() - 1); i >= 0; --i)
					{
						if (this->tokenString[i] == 0xFF)
						{
							this->tokenString[i] = 0;

							if (!i)
							{
								// Prepend here, as /dev/urandom says so ;) https://github.com/IW4x/iw4x-client-node/wikis/technical-information#incrementing-the-token
								this->tokenString = std::basic_string<std::uint8_t>(reinterpret_cast<std::uint8_t*>(const_cast<char*>("\0")), 1) + this->tokenString;
								break;
							}
						}
						else
						{
							++this->tokenString[i];
							break;
						}
					}
				}

				return *this;
			}

			Token operator++ (int)
			{
				Token result = *this;
				this->operator++();
				return result;
			}

			bool operator==(const Token& token) const
			{
				return (this->toString() == token.toString());
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
				else if (this->toString().size() < token.toString().size())
				{
					return true;
				}
				else if (this->toString().size() > token.toString().size())
				{
					return false;
				}
				else
				{
					auto lStr = this->toString();
					auto rStr = token.toString();

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

			std::string toString()
			{
				return std::string(this->tokenString.begin(), this->tokenString.end());
			}

			std::string toString() const
			{
				return std::string(this->tokenString.begin(), this->tokenString.end());
			}

			std::basic_string<uint8_t> toUnsignedString()
			{
				return this->tokenString;
			}

			void clear()
			{
				this->tokenString.clear();
			}

		private:
			std::basic_string<uint8_t> tokenString;
		};

		class Rand
		{
		public:
			static std::string GenerateChallenge();
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
				Key() : keyStorage(new ecc_key)
				{
					ZeroMemory(this->getKeyPtr(), sizeof(*this->getKeyPtr()));
				};
				Key(ecc_key* key) : Key() { if (key) std::memmove(this->getKeyPtr(), key, sizeof(*key)); };
				Key(ecc_key key) : Key(&key) {};
				~Key()
				{
					if (this->keyStorage.use_count() <= 1)
					{
						this->free();
					}
				};

				bool isValid()
				{
					return (!Utils::Memory::IsSet(this->getKeyPtr(), 0, sizeof(*this->getKeyPtr())));
				}

				ecc_key* getKeyPtr()
				{
					return this->keyStorage.get();
				}

				std::string getPublicKey()
				{
					uint8_t buffer[512] = { 0 };
					DWORD length = sizeof(buffer);

					if (ecc_ansi_x963_export(this->getKeyPtr(), buffer, &length) == CRYPT_OK)
					{
						return std::string(reinterpret_cast<char*>(buffer), length);
					}

					return "";
				}

				void set(const std::string& pubKeyBuffer)
				{
					this->free();

					if (ecc_ansi_x963_import(reinterpret_cast<const uint8_t*>(pubKeyBuffer.data()), pubKeyBuffer.size(), this->getKeyPtr()) != CRYPT_OK)
					{
						ZeroMemory(this->getKeyPtr(), sizeof(*this->getKeyPtr()));
					}
				}

				void deserialize(const std::string& key)
				{
					this->free();

					if (ecc_import(reinterpret_cast<const uint8_t*>(key.data()), key.size(), this->getKeyPtr()) != CRYPT_OK)
					{
						ZeroMemory(this->getKeyPtr(), sizeof(*this->getKeyPtr()));
					}
				}

				std::string serialize(int type = PK_PRIVATE)
				{
					uint8_t buffer[4096] = { 0 };
					DWORD length = sizeof(buffer);

					if (ecc_export(buffer, &length, type, this->getKeyPtr()) == CRYPT_OK)
					{
						return std::string(reinterpret_cast<char*>(buffer), length);
					}

					return "";
				}

				void free()
				{
					if (this->isValid())
					{
						ecc_free(this->getKeyPtr());
					}

					ZeroMemory(this->getKeyPtr(), sizeof(*this->getKeyPtr()));
				}

				bool operator==(Key& key)
				{
					return (this->isValid() && key.isValid() && this->serialize(PK_PUBLIC) == key.serialize(PK_PUBLIC));
				}

			private:
				std::shared_ptr<ecc_key> keyStorage;
			};

			static Key GenerateKey(int bits);
			static std::string SignMessage(Key key, const std::string& message);
			static bool VerifyMessage(Key key, const std::string& message, const std::string& signature);
		};

		class RSA
		{
		public:
			class Key
			{
			public:
				Key() : keyStorage(new rsa_key)
				{
					ZeroMemory(this->getKeyPtr(), sizeof(*this->getKeyPtr()));
				}

				Key(rsa_key* key) : Key() { if (key) std::memmove(this->getKeyPtr(), key, sizeof(*key)); }
				Key(rsa_key key) : Key(&key) {}

				~Key()
				{
					if (this->keyStorage.use_count() <= 1)
					{
						this->free();
					}
				}

				rsa_key* getKeyPtr()
				{
					return this->keyStorage.get();
				}

				bool isValid()
				{
					return (!Memory::IsSet(this->getKeyPtr(), 0, sizeof(*this->getKeyPtr())));
				}

				void free()
				{
					if (this->isValid())
					{
						rsa_free(this->getKeyPtr());
					}

					ZeroMemory(this->getKeyPtr(), sizeof(*this->getKeyPtr()));
				}

			private:
				std::shared_ptr<rsa_key> keyStorage;
			};

			static Key GenerateKey(int bits);
			static std::string SignMessage(Key key, const std::string& message);
			static bool VerifyMessage(Key key, const std::string& message, const std::string& signature);
		};

		class DES3
		{
		public:
			static void Initialize();
			static std::string Encrypt(const std::string& text, const std::string& iv, const std::string& key);
			static std::string Decrpyt(const std::string& text, const std::string& iv, const std::string& key);
		};

		class Tiger
		{
		public:
			static std::string Compute(const std::string& data, bool hex = false);
			static std::string Compute(const std::uint8_t* data, std::size_t length, bool hex = false);
		};

		class SHA1
		{
		public:
			static std::string Compute(const std::string& data, bool hex = false);
			static std::string Compute(const std::uint8_t* data, std::size_t length, bool hex = false);
		};

		class SHA256
		{
		public:
			static std::string Compute(const std::string& data, bool hex = false);
			static std::string Compute(const std::uint8_t* data, std::size_t length, bool hex = false);
		};

		class SHA512
		{
		public:
			static std::string Compute(const std::string& data, bool hex = false);
			static std::string Compute(const std::uint8_t* data, std::size_t length, bool hex = false);
		};

		class JenkinsOneAtATime
		{
		public:
			static unsigned int Compute(const std::string& data);
			static unsigned int Compute(const char* key, std::size_t len);
		};
	}
}
