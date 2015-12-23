#include "..\STDInclude.hpp"

namespace Utils
{
	Hook::~Hook()
	{
		if (Hook::Initialized)
		{
			Hook::Uninstall();
		}
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

		DWORD d;
		VirtualProtect(Hook::Place, sizeof(Hook::Buffer), PAGE_EXECUTE_READWRITE, &d);
		memcpy(Hook::Buffer, Hook::Place, sizeof(Hook::Buffer));

		char* Code = (char*)Hook::Place;

		*Code = (char)(Hook::UseJump ? 0xE9 : 0xE8);

		*(size_t*)&Code[1] = (size_t)Hook::Stub - ((size_t)Hook::Place + 5);

		VirtualProtect(Hook::Place, sizeof(Hook::Buffer), d, &d);

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

		DWORD d;
		VirtualProtect(Hook::Place, sizeof(Hook::Buffer), PAGE_EXECUTE_READWRITE, &d);
		
		memcpy(Hook::Place, Hook::Buffer, sizeof(Hook::Buffer));

		VirtualProtect(Hook::Place, sizeof(Hook::Buffer), d, &d);

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
}
