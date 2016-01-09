#include "STDInclude.hpp"

namespace Components
{
	char Theatre::BaselineSnapshot[131072] = { 0 };
	PBYTE Theatre::BaselineSnapshotMsg = 0;
	int Theatre::BaselineSnapshotMsgLen;
	int Theatre::BaselineSnapshotMsgOff;

	void Theatre::GamestateWriteStub(Game::msg_t* msg, char byte)
	{
		Game::MSG_WriteLong(msg, 0);
		Game::MSG_WriteByte(msg, byte);
	}

	void Theatre::RecordGamestateStub()
	{
		int sequence = (*Game::serverMessageSequence - 1);
		Game::FS_Write(&sequence, 4, *Game::demoFile);
	}

	void __declspec(naked) Theatre::BaselineStoreStub()
	{
		// Store snapshot message
		__asm mov Theatre::BaselineSnapshotMsg, edi

		// Store offset and length
		Theatre::BaselineSnapshotMsgLen = *(int*)(Theatre::BaselineSnapshotMsg + 20);
		Theatre::BaselineSnapshotMsgOff = *(int*)(Theatre::BaselineSnapshotMsg + 28) - 7;

		// Copy to our snapshot buffer
		memcpy(Theatre::BaselineSnapshot, *(DWORD**)(Theatre::BaselineSnapshotMsg + 8), *(DWORD*)(Theatre::BaselineSnapshotMsg + 20));

		__asm
		{
			mov edx, 5ABEF5h
			jmp edx
		}
	}

	void Theatre::WriteBaseline()
	{
		static char bufData[131072];
		static char cmpData[131072];

		Game::msg_t buf;

		Game::MSG_Init(&buf, bufData, 131072);
		Game::MSG_WriteData(&buf, &Theatre::BaselineSnapshot[Theatre::BaselineSnapshotMsgOff], Theatre::BaselineSnapshotMsgLen - Theatre::BaselineSnapshotMsgOff);
		Game::MSG_WriteByte(&buf, 6);

		int compressedSize = Game::MSG_WriteBitsCompress(false, buf.data, cmpData, buf.cursize);
		int fileCompressedSize = compressedSize + 4;

		int byte8 = 8;
		char byte0 = 0;

		Game::FS_Write(&byte0, 1, *Game::demoFile);
		Game::FS_Write(Game::serverMessageSequence, 4, *Game::demoFile);
		Game::FS_Write(&fileCompressedSize, 4, *Game::demoFile);
		Game::FS_Write(&byte8, 4, *Game::demoFile);

		for (int i = 0; i < compressedSize; i += 1024)
		{
			int size = min(compressedSize - i, 1024);

			if (i + size >= sizeof(cmpData))
			{
				Logger::Print("Error: Writing compressed demo baseline exceeded buffer\n");
				break;
			}
			
			Game::FS_Write(&cmpData[i], size, *Game::demoFile);
		}
	}

	void __declspec(naked) Theatre::BaselineToFileStub()
	{
		__asm
		{
			call Theatre::WriteBaseline

			// Restore overwritten operation
			mov ecx, 0A5E9C4h
			mov [ecx], 0

			// Return to original code
			mov ecx, 5A863Ah
			jmp ecx
		}
	}

	void __declspec(naked) Theatre::AdjustTimeDeltaStub()
	{
		__asm
		{
			mov eax, Game::demoPlaying
			mov eax, [eax]
			test al, al
			jz continue

			// delta doesn't drift for demos
			retn

		continue:
			mov eax, 5A1AD0h
			jmp eax
		}
	}

	void __declspec(naked) Theatre::ServerTimedOutStub()
	{
		__asm
		{
			mov eax, Game::demoPlaying
			mov eax, [eax]
			test al, al
			jz continue

			mov eax, 5A8E70h
			jmp eax

		continue:
			mov eax, 0B2BB90h
			mov esi, 5A8E08h
			jmp esi
		}
	}

	void __declspec(naked) Theatre::UISetActiveMenuStub()
	{
		if (*Game::demoPlaying == 1)
		{
			__asm
			{
				mov eax, 4CB49Ch
				jmp eax
			}
		}

		__asm
		{
			mov ecx, [esp + 10h]
			push 10h
			push ecx
			mov eax, 4CB3F6h
			jmp eax
		}
	}

	Theatre::Theatre()
	{
		Utils::Hook(0x5A8370, Theatre::GamestateWriteStub, HOOK_CALL).Install()->Quick();
		Utils::Hook(0x5A85D2, Theatre::RecordGamestateStub, HOOK_CALL).Install()->Quick();
		Utils::Hook(0x5ABE36, Theatre::BaselineStoreStub, HOOK_JUMP).Install()->Quick();
		Utils::Hook(0x5A8630, Theatre::BaselineToFileStub, HOOK_JUMP).Install()->Quick();
		Utils::Hook(0x4CB3EF, Theatre::UISetActiveMenuStub, HOOK_JUMP).Install()->Quick();
		Utils::Hook(0x50320E, Theatre::AdjustTimeDeltaStub, HOOK_CALL).Install()->Quick();
		Utils::Hook(0x5A8E03, Theatre::ServerTimedOutStub, HOOK_JUMP).Install()->Quick();

		// set the configstrings stuff to load the default (empty) string table; this should allow demo recording on all gametypes/maps
		if(!Dedicated::IsDedicated()) Utils::Hook::Set<char*>(0x47440B, "mp/defaultStringTable.csv");
	
		*(BYTE*)0x5AC854 = 4;
		*(BYTE*)0x5AC85A = 4;
	}
}
