#include <STDInclude.hpp>
#include "Security.hpp"

namespace Components
{
	int Security::MsgReadBitsCompressCheckSV(const unsigned char* from, unsigned char* to, int size)
	{
		static unsigned char buffer[0x8000];

		if (size > 0x800) return 0;
		size = Game::MSG_ReadBitsCompress(from, buffer, size);

		if (size > 0x800) return 0;
		std::memcpy(to, buffer, size);

		return size;
	}

	int Security::MsgReadBitsCompressCheckCL(const unsigned char* from, unsigned char* to, int size)
	{
		static unsigned char buffer[0x100000];

		if (size > 0x20000) return 0;
		size = Game::MSG_ReadBitsCompress(from, buffer, size);

		if (size > 0x20000) return 0;
		std::memcpy(to, buffer, size);

		return size;
	}

	int Security::SVCanReplaceServerCommand(Game::client_t* /*client*/, const char* /*cmd*/)
	{
		// This is a fix copied from V2. As I don't have time to investigate, let's simply trust them
		return -1;
	}

	long Security::AtolAdjustPlayerLimit(const char* string)
	{
		return std::min<long>(std::atol(string), 18);
	}

	void Security::SelectStringTableEntryInDvarStub()
	{
		Command::ClientParams params;

		if (params.size() >= 4)
		{
			const auto* name = params.get(3);
			// If it's a command don't execute it
			if (Command::Find(name) != nullptr)
			{
				Logger::Debug("CL_SelectStringTableEntryInDvar_f: parameter is a command");
				return;
			}

			const auto* dvar = Game::Dvar_FindVar(name);
			if (!dvar)
			{
				// If it's not a dvar let it continue
				Game::CL_SelectStringTableEntryInDvar_f();
				return;
			}

			constexpr auto disallowedFlags = (Game::DVAR_CHEAT | Game::DVAR_INIT
				| Game::DVAR_ROM | Game::DVAR_EXTERNAL | Game::DVAR_LATCH);

			// If it's a dvar check that it does not have disallowed flags
			if ((dvar->flags & disallowedFlags) != 0)
			{
				Logger::Debug("CL_SelectStringTableEntryInDvar_f: parameter is a protected dvar");
				return;
			}
		}

		Game::CL_SelectStringTableEntryInDvar_f();
	}

	__declspec(naked) int Security::G_GetClientScore()
	{
		__asm
		{
			mov eax, [esp + 4] // index
			mov ecx, ds:1A831A8h // level: &g_clients

			test ecx, ecx
			jz invalid_ptr

			imul eax, 366Ch
			mov eax, [eax + ecx + 3134h]
			ret

		invalid_ptr:
			xor eax, eax
			ret
		}
	}

	void Security::G_LogPrintfStub(const char* fmt)
	{
		Game::G_LogPrintf("%s", fmt);
	}

	void Security::NET_DeferPacketToClientStub(Game::netadr_t* net_from, Game::msg_t* net_message)
	{
		assert(net_from);
		assert(net_message);

		if (static_cast<std::size_t>(net_message->cursize) >= sizeof(Game::DeferredMsg::data))
		{
			Logger::Debug("Dropping net_message. Size is {}", net_message->cursize);
			return;
		}

		auto* msg = &Game::deferredQueue->msgs[Game::deferredQueue->send % std::extent_v<decltype(Game::DeferredQueue::msgs)>];
		std::memcpy(msg->data, net_message->data, net_message->cursize);

		msg->datalen = net_message->cursize;
		msg->addr = *net_from;

		InterlockedIncrement(&Game::deferredQueue->send);
	}

	Security::Security()
	{
		// Exploit fixes
		Utils::Hook(0x414D92, MsgReadBitsCompressCheckSV, HOOK_CALL).install()->quick(); // SV_ExecuteClientCommands
		Utils::Hook(0x4A9F56, MsgReadBitsCompressCheckCL, HOOK_CALL).install()->quick(); // CL_ParseServerMessage
		Utils::Hook(0x407376, SVCanReplaceServerCommand, HOOK_CALL).install()->quick(); // SV_CanReplaceServerCommand

		Utils::Hook::Set<BYTE>(0x412370, 0xC3); // SV_SteamAuthClient
		Utils::Hook::Set<BYTE>(0x5A8C70, 0xC3); // CL_HandleRelayPacket

		Utils::Hook::Nop(0x41698E, 5); // Disable Svcmd_EntityList_f

		// Patch selectStringTableEntryInDvar
		Utils::Hook::Set<void(*)()>(0x405959, SelectStringTableEntryInDvarStub);

		// Patch G_GetClientScore for uninitialized game
		Utils::Hook(0x469AC0, G_GetClientScore, HOOK_JUMP).install()->quick();

		// Requests can be malicious
		Utils::Hook(0x5B67ED, AtolAdjustPlayerLimit, HOOK_CALL).install()->quick(); // PartyHost_HandleJoinPartyRequest

		// Patch unsecure call to G_LogPrint inside GScr_LogPrint
		// This function is unsafe because IW devs forgot to G_LogPrintf("%s", fmt)
		Utils::Hook(0x5F70B5, G_LogPrintfStub, HOOK_CALL).install()->quick();

		// Fix packets causing buffer overflow
		Utils::Hook(0x6267E3, NET_DeferPacketToClientStub, HOOK_CALL).install()->quick();

		// Prevent curl 7_19_4 from running
		// Call to DL_Init from Live_Init
		Utils::Hook::Nop(0x420937, 5);
		// Call to DL_CheckOngoingDownloads from Live_Frame
		Utils::Hook::Nop(0x40F8DB, 5);
		// Call to LiveStorage_FetchPlaylists from Live_Frame
		Utils::Hook::Nop(0x40F88C, 5);
		// Call to LiveStorage_FetchPlaylists from Live_Init
		Utils::Hook::Nop(0x420B54, 5);
	}
}
