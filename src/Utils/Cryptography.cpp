#include <STDInclude.hpp>

/// http://www.opensource.apple.com/source/CommonCrypto/CommonCrypto-55010/Source/libtomcrypt/doc/libTomCryptDoc.pdf

namespace Utils
{
	namespace Cryptography
	{
		void Initialize()
		{
			DES3::Initialize();
			Rand::Initialize();
		}

#pragma region Rand

		prng_state Rand::State;

		std::string Rand::GenerateChallenge()
		{
			char buffer[512]{};
			int pos = 0;

			pos += sprintf_s(&buffer[pos], sizeof(buffer) - pos, "%X", GenerateInt());
			pos += sprintf_s(&buffer[pos], sizeof(buffer) - pos, "%X", ~timeGetTime() ^ GenerateInt());
			pos += sprintf_s(&buffer[pos], sizeof(buffer) - pos, "%X", GenerateInt());

			return std::string{ buffer, static_cast<std::size_t>(pos) };
		}

		std::uint32_t Rand::GenerateInt()
		{
			std::uint32_t number = 0;
			fortuna_read(reinterpret_cast<std::uint8_t*>(&number), sizeof(number), &Rand::State);
			return number;
		}

		void Rand::Initialize()
		{
			ltc_mp = ltm_desc;
			register_prng(&fortuna_desc);
			rng_make_prng(128, find_prng("fortuna"), &Rand::State, nullptr);
		}

#pragma endregion

#pragma region ECC

		ECC::Key ECC::GenerateKey(int bits)
		{
			Key key;

			ltc_mp = ltm_desc;
			register_prng(&sprng_desc);
			ecc_make_key(nullptr, find_prng("sprng"), bits / 8, key.getKeyPtr());

			return key;
		}

		std::string ECC::SignMessage(Key key, const std::string& message)
		{
			if (!key.isValid()) return {};

			std::uint8_t buffer[512];
			unsigned long length = sizeof(buffer);

			ltc_mp = ltm_desc;
			register_prng(&sprng_desc);
			ecc_sign_hash(reinterpret_cast<const std::uint8_t*>(message.data()), message.size(), buffer, &length, nullptr, find_prng("sprng"), key.getKeyPtr());

			return  std::string{ reinterpret_cast<char*>(buffer), length };
		}

		bool ECC::VerifyMessage(Key key, const std::string& message, const std::string& signature)
		{
			if (!key.isValid()) return false;

			ltc_mp = ltm_desc;

			int result = 0;
			return (ecc_verify_hash(reinterpret_cast<const std::uint8_t*>(signature.data()), signature.size(),
			                        reinterpret_cast<const std::uint8_t*>(message.data()), message.size(),
			                        &result, key.getKeyPtr()) == CRYPT_OK && result != 0);
		}

#pragma endregion

#pragma region RSA

		RSA::Key RSA::GenerateKey(int bits)
		{
			Key key;

			register_prng(&sprng_desc);
			register_hash(&sha1_desc);

			ltc_mp = ltm_desc;

			rsa_make_key(nullptr, find_prng("sprng"), bits / 8, 65537, key.getKeyPtr());

			return key;
		}

		std::string RSA::SignMessage(Key key, const std::string& message)
		{
			if (!key.isValid()) return {};

			std::uint8_t buffer[512];
			unsigned long length = sizeof(buffer);

			register_prng(&sprng_desc);
			register_hash(&sha1_desc);

			ltc_mp = ltm_desc;

			rsa_sign_hash(reinterpret_cast<const std::uint8_t*>(message.data()), message.size(), buffer, &length, NULL, find_prng("sprng"), find_hash("sha1"), 0, key.getKeyPtr());

			return  std::string{ reinterpret_cast<char*>(buffer), length };
		}

		bool RSA::VerifyMessage(Key key, const std::string& message, const std::string& signature)
		{
			if (!key.isValid()) return false;

			register_hash(&sha1_desc);

			ltc_mp = ltm_desc;

			int result = 0;
			return (rsa_verify_hash(reinterpret_cast<const std::uint8_t*>(signature.data()), signature.size(), reinterpret_cast<const std::uint8_t*>(message.data()), message.size(), find_hash("sha1"), 0, &result, key.getKeyPtr()) == CRYPT_OK && result != 0);
		}

#pragma endregion

#pragma region DES3

		void DES3::Initialize()
		{
			register_cipher(&des3_desc);
		}

		std::string DES3::Encrypt(const std::string& text, const std::string& iv, const std::string& key)
		{
			std::string encData;
			encData.resize(text.size());

			symmetric_CBC cbc;
			int des3 = find_cipher("3des");

			cbc_start(des3, reinterpret_cast<const std::uint8_t*>(iv.data()), reinterpret_cast<const std::uint8_t*>(key.data()), key.size(), 0, &cbc);
			cbc_encrypt(reinterpret_cast<const std::uint8_t*>(text.data()), reinterpret_cast<uint8_t*>(encData.data()), text.size(), &cbc);
			cbc_done(&cbc);

			return encData;
		}

		std::string DES3::Decrpyt(const std::string& data, const std::string& iv, const std::string& key)
		{
			std::string decData;
			decData.resize(data.size());

			symmetric_CBC cbc;
			int des3 = find_cipher("3des");

			cbc_start(des3, reinterpret_cast<const std::uint8_t*>(iv.data()), reinterpret_cast<const std::uint8_t*>(key.data()), key.size(), 0, &cbc);
			cbc_decrypt(reinterpret_cast<const std::uint8_t*>(data.data()), reinterpret_cast<std::uint8_t*>(decData.data()), data.size(), &cbc);
			cbc_done(&cbc);

			return decData;
		}

#pragma endregion

#pragma region Tiger

		std::string Tiger::Compute(const std::string& data, bool hex)
		{
			return Compute(reinterpret_cast<const std::uint8_t*>(data.data()), data.size(), hex);
		}

		std::string Tiger::Compute(const std::uint8_t* data, std::size_t length, bool hex)
		{
			std::uint8_t buffer[24]{};

			hash_state state;
			tiger_init(&state);
			tiger_process(&state, data, length);
			tiger_done(&state, buffer);

			std::string hash{ reinterpret_cast<char*>(buffer), sizeof(buffer) };
			if (!hex) return hash;

			return String::DumpHex(hash, {});
		}

#pragma endregion

#pragma region SHA1

		std::string SHA1::Compute(const std::string& data, bool hex)
		{
			return Compute(reinterpret_cast<const std::uint8_t*>(data.data()), data.size(), hex);
		}

		std::string SHA1::Compute(const std::uint8_t* data, std::size_t length, bool hex)
		{
			std::uint8_t buffer[20]{};

			hash_state state;
			sha1_init(&state);
			sha1_process(&state, data, length);
			sha1_done(&state, buffer);

			std::string hash{ reinterpret_cast<char*>(buffer), sizeof(buffer) };
			if (!hex) return hash;

			return String::DumpHex(hash, {});
		}

#pragma endregion

#pragma region SHA256

		std::string SHA256::Compute(const std::string& data, bool hex)
		{
			return Compute(reinterpret_cast<const std::uint8_t*>(data.data()), data.size(), hex);
		}

		std::string SHA256::Compute(const std::uint8_t* data, std::size_t length, bool hex)
		{
			std::uint8_t buffer[32]{};

			hash_state state;
			sha256_init(&state);
			sha256_process(&state, data, length);
			sha256_done(&state, buffer);

			std::string hash{ reinterpret_cast<char*>(buffer), sizeof(buffer) };
			if (!hex) return hash;

			return String::DumpHex(hash, {});
		}

#pragma endregion

#pragma region SHA512

		std::string SHA512::Compute(const std::string& data, bool hex)
		{
			return Compute(reinterpret_cast<const std::uint8_t*>(data.data()), data.size(), hex);
		}

		std::string SHA512::Compute(const std::uint8_t* data, std::size_t length, bool hex)
		{
			std::uint8_t buffer[64]{};

			hash_state state;
			sha512_init(&state);
			sha512_process(&state, data, length);
			sha512_done(&state, buffer);

			std::string hash{ reinterpret_cast<char*>(buffer), sizeof(buffer) };
			if (!hex) return hash;

			return String::DumpHex(hash, {});
		}

#pragma endregion

#pragma region JenkinsOneAtATime

		unsigned int JenkinsOneAtATime::Compute(const std::string& data)
		{
			return Compute(data.data(), data.size());
		}

		unsigned int JenkinsOneAtATime::Compute(const char* key, std::size_t len)
		{
			unsigned int hash, i;
			for (hash = i = 0; i < len; ++i)
			{
				hash += key[i];
				hash += (hash << 10);
				hash ^= (hash >> 6);
			}
			hash += (hash << 3);
			hash ^= (hash >> 11);
			hash += (hash << 15);
			return hash;
		}

#pragma endregion
	}
}
