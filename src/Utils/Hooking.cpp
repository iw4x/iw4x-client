#include "STDInclude.hpp"

namespace Utils
{
	std::map<void*, void*> Hook::Interceptor::IReturn;
	std::map<void*, void(*)()> Hook::Interceptor::ICallbacks;

	void Hook::Interceptor::Install(void* place, void(*stub)())
	{
		return Hook::Interceptor::Install(reinterpret_cast<void**>(place), stub);
	}

	void Hook::Interceptor::Install(void** place, void(*stub)())
	{
		Hook::Interceptor::IReturn[place] = *place;
		Hook::Interceptor::ICallbacks[place] = stub;
		*place = Hook::Interceptor::InterceptionStub;
	}

	void __declspec(naked) Hook::Interceptor::InterceptionStub()
	{
		__asm
		{
			sub esp, 4h          // Reserve space on the stack for the return address
			pushad               // Store registers

			lea eax, [esp + 20h] // Load initial stack pointer
			push eax             // Push it onto the stack

			call RunCallback     // Run the callback based on the given stack pointer
			call PopReturn       // Get the initial return address according to the stack pointer

			add esp, 4h          // Clear the stack

			mov [esp + 20h], eax // Store the return address at the reserved space
			popad                // Restore the registers

			retn                 // Return (jump to our return address)
		}
	}

	void Hook::Interceptor::RunCallback(void* place)
	{
		auto iCallback = Hook::Interceptor::ICallbacks.find(place);
		if (iCallback != Hook::Interceptor::ICallbacks.end())
		{
			iCallback->second();
			Hook::Interceptor::ICallbacks.erase(iCallback);
		}
	}

	void* Hook::Interceptor::PopReturn(void* place)
	{
		void* retVal = nullptr;

		auto iReturn = Hook::Interceptor::IReturn.find(place);
		if (iReturn != Hook::Interceptor::IReturn.end())
		{
			retVal = iReturn->second;
			Hook::Interceptor::IReturn.erase(iReturn);
		}

		return retVal;
	}

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

	Hook* Hook::Install(bool unprotect, bool keepUnportected)
	{
		Hook::StateMutex.lock();

		if (!Hook::Initialized || Hook::Installed)
		{
			Hook::StateMutex.unlock();
			return this;
		}

		Hook::Installed = true;

		if (unprotect) VirtualProtect(Hook::Place, sizeof(Hook::Buffer), PAGE_EXECUTE_READWRITE, &this->Protection);
		memcpy(Hook::Buffer, Hook::Place, sizeof(Hook::Buffer));

		char* code = static_cast<char*>(Hook::Place);

		*code = static_cast<char>(Hook::UseJump ? 0xE9 : 0xE8);

		*reinterpret_cast<size_t*>(code + 1) = reinterpret_cast<size_t>(Hook::Stub) - (reinterpret_cast<size_t>(Hook::Place) + 5);

		if (unprotect && !keepUnportected) VirtualProtect(Hook::Place, sizeof(Hook::Buffer), Hook::Protection, &this->Protection);

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

	Hook* Hook::Uninstall(bool unprotect)
	{
		Hook::StateMutex.lock();

		if (!Hook::Initialized || !Hook::Installed)
		{
			Hook::StateMutex.unlock();
			return this;
		}

		Hook::Installed = false;

		if(unprotect) VirtualProtect(Hook::Place, sizeof(Hook::Buffer), PAGE_EXECUTE_READWRITE, &this->Protection);
		
		memcpy(Hook::Place, Hook::Buffer, sizeof(Hook::Buffer));

		if (unprotect) VirtualProtect(Hook::Place, sizeof(Hook::Buffer), Hook::Protection, &this->Protection);

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
		DWORD oldProtect;
		VirtualProtect(place, length, PAGE_EXECUTE_READWRITE, &oldProtect);

		memset(place, 0x90, length);

		VirtualProtect(place, length, oldProtect, &oldProtect);
		FlushInstructionCache(GetCurrentProcess(), place, length);
	}

	void Hook::Nop(DWORD place, size_t length)
	{ 
		Nop(reinterpret_cast<void*>(place), length);
	}

	void Hook::SetString(void* place, const char* string, size_t length)
	{
		DWORD oldProtect;
		VirtualProtect(place, length + 1, PAGE_EXECUTE_READWRITE, &oldProtect);

		strncpy(static_cast<char*>(place), string, length);

		VirtualProtect(place, length + 1, oldProtect, &oldProtect);
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

	void Hook::RedirectJump(void* place, void* stub)
	{
		char* operandPtr = static_cast<char*>(place) + 2;
		int newOperand = reinterpret_cast<int>(stub) - (reinterpret_cast<int>(place) + 6);
		Utils::Hook::Set<int>(operandPtr, newOperand);
	}

	void Hook::RedirectJump(DWORD place, void* stub)
	{
		Hook::RedirectJump(reinterpret_cast<void*>(place), stub);
	}
}
