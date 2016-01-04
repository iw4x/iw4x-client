#include "STDInclude.hpp"

namespace Utils
{
	Hook::~Hook()
	{
		if (Hook::Initialized)
		{
			Hook::Uninstall();
		}
	}

	Hook* Hook::Initialize(DWORD place, void(*stub)(), bool useJump)
	{
		return Hook::Initialize(place, (void*)stub, useJump);
	}

	Hook* Hook::Initialize(DWORD place, void* stub, bool useJump)
	{
		return Hook::Initialize((void*)place, stub, useJump);
	}

	Hook* Hook::Initialize(void* place, void* stub, bool useJump)
	{
		if (Hook::Initialized) return this;
		Hook::Initialized = true;

		Hook::UseJump = useJump;
		Hook::Place = place;
		Hook::Stub = stub;

		Hook::Original = (char*)Hook::Place + 5 + *(DWORD*)((DWORD)Hook::Place + 1);

		return this;
	}

	Hook* Hook::Install()
	{
		Hook::StateMutex.lock();

		if (!Hook::Initialized || Hook::Installed)
		{
			Hook::StateMutex.unlock();
			return this;
		}

		Hook::Installed = true;

		DWORD d = 1;
		VirtualProtect(Hook::Place, sizeof(Hook::Buffer), PAGE_EXECUTE_READWRITE, &this->Protection);
		memcpy(Hook::Buffer, Hook::Place, sizeof(Hook::Buffer));

		char* Code = (char*)Hook::Place;

		*Code = (char)(Hook::UseJump ? 0xE9 : 0xE8);

		*(size_t*)&Code[1] = (size_t)Hook::Stub - ((size_t)Hook::Place + 5);

		VirtualProtect(Hook::Place, sizeof(Hook::Buffer), Hook::Protection, &this->Protection);

		FlushInstructionCache(GetCurrentProcess(), Hook::Place, sizeof(Hook::Buffer));

		Hook::StateMutex.unlock();

		return this;
	}

	void Hook::Quick()
	{
		if (Hook::Installed)
		{
			Hook::Installed = false;
		}
	}

	Hook* Hook::Uninstall()
	{
		Hook::StateMutex.lock();

		if (!Hook::Initialized || !Hook::Installed)
		{
			Hook::StateMutex.unlock();
			return this;
		}

		Hook::Installed = false;

		VirtualProtect(Hook::Place, sizeof(Hook::Buffer), PAGE_EXECUTE_READWRITE, &this->Protection);
		
		memcpy(Hook::Place, Hook::Buffer, sizeof(Hook::Buffer));

		VirtualProtect(Hook::Place, sizeof(Hook::Buffer), Hook::Protection, &this->Protection);

		FlushInstructionCache(GetCurrentProcess(), Hook::Place, sizeof(Hook::Buffer));

		Hook::StateMutex.unlock();

		return this;
	}

	void* Hook::GetAddress()
	{
		return Hook::Place;
	}

	void Hook::Nop(void* place, size_t length)
	{ 
		memset(place, 0x90, length);
	}

	void Hook::Nop(DWORD place, size_t length)
	{ 
		Nop((void*)place, length);
	}

	void Hook::SetString(void* place, const char* string, size_t length)
	{
		strncpy((char*)place, string, length);
	}

	void Hook::SetString(DWORD place, const char* string, size_t length)
	{
		Hook::SetString((void*)place, string, length);
	}

	void Hook::SetString(void* place, const char* string)
	{
		Hook::SetString(place, string, strlen((char*)place));
	}

	void Hook::SetString(DWORD place, const char* string)
	{
		Hook::SetString((void*)place, string);
	}
}
