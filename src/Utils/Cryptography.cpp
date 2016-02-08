#include "STDInclude.hpp"

/// http://www.opensource.apple.com/source/CommonCrypto/CommonCrypto-55010/Source/libtomcrypt/doc/libTomCryptDoc.pdf

namespace Utils
{
	namespace Cryptography
	{
#pragma region Rand

		uint32_t Rand::GenerateInt()
		{
			size_t length = 0;
			uint8_t buffer[4] = { 0 };

			while (length != 4)
			{
				length = sprng_read(buffer, sizeof(buffer), NULL);
			}

			return *reinterpret_cast<uint32_t*>(buffer);
		}

#pragma endregion

#pragma region ECDSA

		ECDSA::Key ECDSA::GenerateKey(int bits)
		{
			ECDSA::Key key;

			register_prng(&sprng_desc);
			register_hash(&sha1_desc);

			ltc_mp = ltm_desc;

			ecc_make_key(NULL, find_prng("sprng"), bits / 8, key.GetKeyPtr());

			return key;
		}

		std::string ECDSA::SignMessage(Key key, std::string message)
		{
			uint8_t buffer[0x200]; // Default size is 512
			DWORD length = sizeof(buffer);

			register_prng(&sprng_desc);

			ltc_mp = ltm_desc;

			ecc_sign_hash(reinterpret_cast<const uint8_t*>(message.data()), message.size(), buffer, &length, NULL, find_prng("sprng"), key.GetKeyPtr());

			return std::string(reinterpret_cast<char*>(buffer), length);
		}

		bool ECDSA::VerifyMessage(Key key, std::string message, std::string signature)
		{
			ltc_mp = ltm_desc;

			int result = 0;
			return (ecc_verify_hash(reinterpret_cast<const uint8_t*>(signature.data()), signature.size(), reinterpret_cast<const uint8_t*>(message.data()), message.size(), &result, key.GetKeyPtr()) == CRYPT_OK && result != 0);
		}

#pragma endregion

#pragma region RSA

		RSA::Key RSA::GenerateKey(int bits)
		{
			RSA::Key key;

			register_prng(&sprng_desc);
			register_hash(&sha1_desc);

			ltc_mp = ltm_desc;

			rsa_make_key(NULL, find_prng("sprng"), bits / 8, 65537, key.GetKeyPtr());

			return key;
		}

		std::string RSA::SignMessage(RSA::Key key, std::string message)
		{
			uint8_t buffer[0x200]; // Default size is 512
			DWORD length = sizeof(buffer);

			register_prng(&sprng_desc);
			register_hash(&sha1_desc);

			ltc_mp = ltm_desc;
			
			rsa_sign_hash(reinterpret_cast<const uint8_t*>(message.data()), message.size(), buffer, &length, NULL, find_prng("sprng"), find_hash("sha1"), 0, key.GetKeyPtr());

			return std::string(reinterpret_cast<char*>(buffer), length);
		}

		bool RSA::VerifyMessage(Key key, std::string message, std::string signature)
		{
			register_hash(&sha1_desc);

			ltc_mp = ltm_desc;

			int result = 0;
			return (rsa_verify_hash(reinterpret_cast<const uint8_t*>(signature.data()), signature.size(), reinterpret_cast<const uint8_t*>(message.data()), message.size(), find_hash("sha1"), 0, &result, key.GetKeyPtr()) == CRYPT_OK && result != 0);
		}
	}

#pragma endregion

}
