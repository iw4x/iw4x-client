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
		return Hook::Initialize(place, reinterpret_cast<void*>(stub), useJump);
	}

	Hook* Hook::Initialize(DWORD place, void* stub, bool useJump)
	{
		return Hook::Initialize(reinterpret_cast<void*>(place), stub, useJump);
	}

	Hook* Hook::Initialize(void* place, void* stub, bool useJump)
	{
		if (Hook::Initialized) return this;
		Hook::Initialized = true;

		Hook::UseJump = useJump;
		Hook::Place = place;
		Hook::Stub = stub;

		Hook::Original = static_cast<char*>(Hook::Place) + 5 + *reinterpret_cast<DWORD*>((static_cast<char*>(Hook::Place) + 1));

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

		VirtualProtect(Hook::Place, sizeof(Hook::Buffer), PAGE_EXECUTE_READWRITE, &this->Protection);
		memcpy(Hook::Buffer, Hook::Place, sizeof(Hook::Buffer));

		char* code = static_cast<char*>(Hook::Place);

		*code = static_cast<char>(Hook::UseJump ? 0xE9 : 0xE8);

		*reinterpret_cast<size_t*>(code + 1) = reinterpret_cast<size_t>(Hook::Stub) - (reinterpret_cast<size_t>(Hook::Place) + 5);

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
		Nop(reinterpret_cast<void*>(place), length);
	}

	void Hook::SetString(void* place, const char* string, size_t length)
	{
		strncpy(static_cast<char*>(place), string, length);
	}

	void Hook::SetString(DWORD place, const char* string, size_t length)
	{
		Hook::SetString(reinterpret_cast<void*>(place), string, length);
	}

	void Hook::SetString(void* place, const char* string)
	{
		Hook::SetString(place, string, strlen(static_cast<char*>(place)));
	}

	void Hook::SetString(DWORD place, const char* string)
	{
		Hook::SetString(reinterpret_cast<void*>(place), string);
	}
}
