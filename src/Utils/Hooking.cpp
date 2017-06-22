#include "STDInclude.hpp"

namespace Utils
{
	std::map<void*, void*> Hook::Interceptor::IReturn;
	std::map<void*, void(*)()> Hook::Interceptor::ICallbacks;

	void Hook::Signature::process()
	{
		if (this->signatures.empty()) return;

		char* _start = reinterpret_cast<char*>(this->start);

		unsigned int sigCount = this->signatures.size();
		Hook::Signature::Container* containers = this->signatures.data();

		for (size_t i = 0; i < this->length; ++i)
		{
			char* address = _start + i;

			for (unsigned int k = 0; k < sigCount; ++k)
			{
				Hook::Signature::Container* container = &containers[k];

				unsigned int j;
				for (j = 0; j < strlen(container->mask); ++j)
				{
					if (container->mask[j] != '?' &&container->signature[j] != address[j])
					{
						break;
					}
				}

				if (j == strlen(container->mask))
				{
					container->callback(address);
				}
			}
		}
	}

	void Hook::Signature::add(Hook::Signature::Container& container)
	{
		Hook::Signature::signatures.push_back(container);
	}

	void Hook::Interceptor::Install(void* place, void* stub)
	{
		return Hook::Interceptor::Install(place, static_cast<void(*)()>(stub));
	}

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

	__declspec(naked) void Hook::Interceptor::InterceptionStub()
	{
		__asm
		{
			sub esp, 4h                             // Reserve space on the stack for the return address
			pushad                                  // Store registers

			lea eax, [esp + 20h]                    // Load initial stack pointer
			push eax                                // Push it onto the stack

			call Hook::Interceptor::RunCallback     // Run the callback based on the given stack pointer
			call Hook::Interceptor::PopReturn       // Get the initial return address according to the stack pointer

			add esp, 4h                             // Clear the stack

			mov [esp + 20h], eax                    // Store the return address at the reserved space
			popad                                   // Restore the registers

			retn                                    // Return (jump to our return address)
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

	void* Hook::Interceptor::PopReturn(void* _place)
	{
		void* retVal = nullptr;

		auto iReturn = Hook::Interceptor::IReturn.find(_place);
		if (iReturn != Hook::Interceptor::IReturn.end())
		{
			retVal = iReturn->second;
			Hook::Interceptor::IReturn.erase(iReturn);
		}

		return retVal;
	}

	Hook::~Hook()
	{
		if (this->initialized)
		{
			this->uninstall();
		}
	}

	Hook* Hook::initialize(DWORD _place, void(*_stub)(), bool _useJump)
	{
		return this->initialize(_place, reinterpret_cast<void*>(_stub), _useJump);
	}

	Hook* Hook::initialize(DWORD _place, void* _stub, bool _useJump)
	{
		return this->initialize(reinterpret_cast<void*>(_place), _stub, _useJump);
	}

	Hook* Hook::initialize(void* _place, void* _stub, bool _useJump)
	{
		if (this->initialized) return this;
		this->initialized = true;

		this->useJump = _useJump;
		this->place = _place;
		this->stub = _stub;

		this->original = static_cast<char*>(this->place) + 5 + *reinterpret_cast<DWORD*>((static_cast<char*>(this->place) + 1));

		return this;
	}

	Hook* Hook::install(bool unprotect, bool keepUnprotected)
	{
		std::lock_guard<std::mutex> _(this->stateMutex);

		if (!this->initialized || this->installed)
		{
			return this;
		}

		this->installed = true;

		if (unprotect) VirtualProtect(this->place, sizeof(this->buffer), PAGE_EXECUTE_READWRITE, &this->protection);
		std::memcpy(this->buffer, this->place, sizeof(this->buffer));

		char* code = static_cast<char*>(this->place);

		*code = static_cast<char>(this->useJump ? 0xE9 : 0xE8);

		*reinterpret_cast<size_t*>(code + 1) = reinterpret_cast<size_t>(this->stub) - (reinterpret_cast<size_t>(this->place) + 5);

		if (unprotect && !keepUnprotected) VirtualProtect(this->place, sizeof(this->buffer), this->protection, &this->protection);

		FlushInstructionCache(GetCurrentProcess(), this->place, sizeof(this->buffer));

		return this;
	}

	void Hook::quick()
	{
		if (Hook::installed)
		{
			Hook::installed = false;
		}
	}

	Hook* Hook::uninstall(bool unprotect)
	{
		std::lock_guard<std::mutex> _(this->stateMutex);

		if (!this->initialized || !this->installed)
		{
			return this;
		}

		this->installed = false;

		if (unprotect) VirtualProtect(this->place, sizeof(this->buffer), PAGE_EXECUTE_READWRITE, &this->protection);

		std::memcpy(this->place, this->buffer, sizeof(this->buffer));

		if (unprotect) VirtualProtect(this->place, sizeof(this->buffer), this->protection, &this->protection);

		FlushInstructionCache(GetCurrentProcess(), this->place, sizeof(this->buffer));

		return this;
	}

	void* Hook::getAddress()
	{
		return this->place;
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

		strncpy_s(static_cast<char*>(place), length, string, length);

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
