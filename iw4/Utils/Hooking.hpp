#pragma once

#define HOOK_JUMP true
#define HOOK_CALL false

namespace Utils
{
	class Hook
	{
	public:
		Hook() : Place(nullptr), Stub(nullptr), Initialized(false), Installed(false), UseJump(false) { ZeroMemory(Hook::Buffer, sizeof(Hook::Buffer)); }
		Hook(void* place, void* stub, bool useJump = true) : Hook() { Hook::Initialize(place, stub, useJump); }
		Hook(DWORD place, void* stub, bool useJump = true) : Hook((void*)place, stub, useJump) {}

		~Hook();

		Hook* Initialize(void* place, void* stub, bool useJump = true);
		Hook* Initialize(DWORD place, void* stub, bool useJump = true);
		Hook* Install();
		Hook* Uninstall();

		void* GetAddress();
		void Quick();

		static void Nop(void* place, size_t length);
		static void Nop(DWORD place, size_t length);

		template <typename T> static void Set(void* place, T value)
		{
			*(T*)place = value;
			FlushInstructionCache(GetCurrentProcess(), place, sizeof(T));
		}

		template <typename T> static void Set(DWORD place, T value)
		{
			return Set<T>((void*)place, value);
		}

		template <typename T> static T Get(void* place)
		{
			return *(T*)place;
		}

		template <typename T> static T Get(DWORD place)
		{
			return Get<T>((void*)place);
		}

	private:
		bool Initialized;
		bool Installed;

		void* Place;
		void* Stub;
		char Buffer[5];
		bool UseJump;

		std::mutex StateMutex;
	};
}
