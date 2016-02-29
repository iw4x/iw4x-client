#include "STDInclude.hpp"

namespace Components
{
	int AntiCheat::LastCheck;
	std::string AntiCheat::Hash;

	void __declspec(naked) AntiCheat::CrashClient()
	{
		static uint8_t crashProcedure[] =
		{
			// Uninstall minidump handler
			0xB8, 0x63, 0xE7, 0x2F, 0x00,           // mov  eax, 2FE763h
			0x05, 0xAD, 0xAD, 0x3C, 0x00,           // add  eax, 3CADADh
			0x6A, 0x58,                             // push 88
			0x8B, 0x80, 0xEA, 0x01, 0x00, 0x00,     // mov  eax, [eax + 1EAh]
			0xFF, 0x10,                             // call dword ptr [eax]

			// Crash me.
			0xB8, 0x4F, 0x91, 0x27, 0x00,           // mov  eax, 27914Fh
			0x05, 0xDD, 0x28, 0x1A, 0x00,           // add  eax, 1A28DDh
			0x80, 0x00, 0x68,                       // add  byte ptr [eax], 68h
			0xC3,                                   // retn
		};

		__asm
		{
			// This does absolutely nothing :P
			// TODO: Obfuscate even more
			xor eax, eax
			mov ebx, [esp + 4h]
			shl ebx, 4h

			// Call our crash procedure
			push offset crashProcedure
			retn
		}
	}

	// This has to be called when doing .text changes during runtime
	void AntiCheat::EmptyHash()
	{
		AntiCheat::LastCheck = 0;
		AntiCheat::Hash.clear();
	}

	void AntiCheat::Frame()
	{
		// Perform check only every 30 seconds
		if (AntiCheat::LastCheck && (Game::Com_Milliseconds() - AntiCheat::LastCheck) < 1000 * 30) return;
		AntiCheat::LastCheck = Game::Com_Milliseconds();

		// Get base module
		std::string hash = Utils::Cryptography::SHA512::Compute(reinterpret_cast<uint8_t*>(GetModuleHandle(NULL)) + 0x1000, 0x2D6000, false);

		// Set the hash, if none is set
		if (AntiCheat::Hash.empty())
		{
			AntiCheat::Hash = hash;
		}
		// Crash if the hashes don't match
		else if(AntiCheat::Hash != hash)
		{
			AntiCheat::CrashClient();
		}
	}

	AntiCheat::AntiCheat()
	{
		AntiCheat::EmptyHash();

		Renderer::OnFrame(AntiCheat::Frame);
		Dedicated::OnFrame(AntiCheat::Frame);

#ifdef DEBUG
		Command::Add("penis", [] (Command::Params)
		{
			AntiCheat::CrashClient();
		});
#endif
	}

	AntiCheat::~AntiCheat()
	{
		AntiCheat::EmptyHash();
	}
}
