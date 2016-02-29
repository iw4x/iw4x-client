#include "STDInclude.hpp"

/// http://www.opensource.apple.com/source/CommonCrypto/CommonCrypto-55010/Source/libtomcrypt/doc/libTomCryptDoc.pdf

namespace Utils
{
	namespace Cryptography
	{

#pragma region Rand

		prng_state Rand::State;

		uint32_t Rand::GenerateInt()
		{
			uint32_t number = 0;
			fortuna_read(reinterpret_cast<uint8_t*>(&number), sizeof(number), &Rand::State);
			return number;
		}

		void Rand::Initialize()
		{
			ltc_mp = ltm_desc;
			register_prng(&fortuna_desc);
			rng_make_prng(128, find_prng("fortuna"), &Rand::State, NULL);
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

#pragma endregion

#pragma region SHA256

		std::string SHA256::Compute(std::string data, bool hex)
		{
			return SHA256::Compute(reinterpret_cast<const uint8_t*>(data.data()), data.size(), hex);
		}

		std::string SHA256::Compute(const uint8_t* data, size_t length, bool hex)
		{
			uint8_t buffer[32] = { 0 };

			hash_state state;
			sha256_init(&state);
			sha256_process(&state, data, length);
			sha256_done(&state, buffer);

			std::string hash(reinterpret_cast<char*>(buffer), sizeof(buffer));
			if (!hex) return hash;

			return Utils::DumpHex(hash, "");
		}

#pragma endregion

#pragma region SHA512

		std::string SHA512::Compute(std::string data, bool hex)
		{
			return SHA512::Compute(reinterpret_cast<const uint8_t*>(data.data()), data.size(), hex);
		}

		std::string SHA512::Compute(const uint8_t* data, size_t length, bool hex)
		{
			uint8_t buffer[64] = { 0 };

			hash_state state;
			sha512_init(&state);
			sha512_process(&state, data, length);
			sha512_done(&state, buffer);

			std::string hash(reinterpret_cast<char*>(buffer), sizeof(buffer));
			if (!hex) return hash;

			return Utils::DumpHex(hash, "");
		}

#pragma endregion

	}
}
