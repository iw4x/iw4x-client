#include "STDInclude.hpp"

/// http://www.opensource.apple.com/source/CommonCrypto/CommonCrypto-55010/Source/libtomcrypt/doc/libTomCryptDoc.pdf

namespace Utils
{
	namespace Cryptography
	{
#pragma region Rand

		uint32_t Rand::GenerateInt()
		{
			static int length = 0;
			static int time = 0;
			static int access = 0;
			static uint8_t randPool[2048] = { 0 };

			int mseconds = Game::Com_Milliseconds();

			if ((mseconds - time) > (1000 * 60 * 5) || (access > (sizeof(randPool) * 30)))
			{
				access = 0;
				length = 0;
			}

			while (length != sizeof(randPool))
			{
				length += sprng_read(randPool, sizeof(randPool), NULL);
			}

			uint8_t numberBuf[4] = { 0 };

			for (int i = 0; i < sizeof(numberBuf); ++i)
			{
				numberBuf[i] = randPool[(rand() + mseconds + i + numberBuf[(i > 0 ? (i - 1) : 0)]) % sizeof(randPool)];
			}

			uint32_t num = *reinterpret_cast<uint32_t*>(numberBuf);

			if (!(access % 100))
			{
				srand(num);
			}

			access++;
			return num;
		}

#pragma endregion

#pragma region ECDSA

		ECDSA::Key ECDSA::GenerateKey(int bits)
		{
			ECDSA::Key key;

			register_prng(&sprng_desc);

			ltc_mp = ltm_desc;

			ecc_make_key(NULL, find_prng("sprng"), bits / 8, key.GetKeyPtr());

			return key;
		}

		std::string ECDSA::SignMessage(Key key, std::string message)
		{
			if (!key.IsValid()) return "";

			uint8_t buffer[512];
			DWORD length = sizeof(buffer);

			register_prng(&sprng_desc);

			ltc_mp = ltm_desc;

			ecc_sign_hash(reinterpret_cast<const uint8_t*>(message.data()), message.size(), buffer, &length, NULL, find_prng("sprng"), key.GetKeyPtr());

			return std::string(reinterpret_cast<char*>(buffer), length);
		}

		bool ECDSA::VerifyMessage(Key key, std::string message, std::string signature)
		{
			if (!key.IsValid()) return false;

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
			if (!key.IsValid()) return "";

			uint8_t buffer[512];
			DWORD length = sizeof(buffer);

			register_prng(&sprng_desc);
			register_hash(&sha1_desc);

			ltc_mp = ltm_desc;
			
			rsa_sign_hash(reinterpret_cast<const uint8_t*>(message.data()), message.size(), buffer, &length, NULL, find_prng("sprng"), find_hash("sha1"), 0, key.GetKeyPtr());

			return std::string(reinterpret_cast<char*>(buffer), length);
		}

		bool RSA::VerifyMessage(Key key, std::string message, std::string signature)
		{
			if (!key.IsValid()) return false;

			register_hash(&sha1_desc);

			ltc_mp = ltm_desc;

			int result = 0;
			return (rsa_verify_hash(reinterpret_cast<const uint8_t*>(signature.data()), signature.size(), reinterpret_cast<const uint8_t*>(message.data()), message.size(), find_hash("sha1"), 0, &result, key.GetKeyPtr()) == CRYPT_OK && result != 0);
		}
	}

#pragma endregion

}
