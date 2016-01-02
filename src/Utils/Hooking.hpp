#pragma once

#define HOOK_JUMP true
#define HOOK_CALL false

namespace Utils
{
	class Hook
	{
	public:
		Hook() : Place(nullptr), Stub(nullptr), Initialized(false), Installed(false), UseJump(false), Protection(0) { ZeroMemory(Hook::Buffer, sizeof(Hook::Buffer)); }
		Hook(void* place, void* stub, bool useJump = true) : Hook() { Hook::Initialize(place, stub, useJump); }
		Hook(DWORD place, void* stub, bool useJump = true) : Hook((void*)place, stub, useJump) {}

		~Hook();

		Hook* Initialize(void* place, void* stub, bool useJump = true);
		Hook* Initialize(DWORD place, void* stub, bool useJump = true);
		Hook* Initialize(DWORD place, void(*stub)(), bool useJump = true); // For lambdas
		Hook* Install();
		Hook* Uninstall();

		void* GetAddress();
		void Quick();

		template <typename T> static std::function<T> Call(DWORD function)
		{
			return std::function<T>((T*)function);
		}

		static void SetString(void* place, const char* string, size_t length);
		static void SetString(DWORD place, const char* string, size_t length);

		static void SetString(void* place, const char* string);
		static void SetString(DWORD place, const char* string);

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

		template <typename T> static void Xor(void* place, T value)
		{
			*(T*)place ^= value;
			FlushInstructionCache(GetCurrentProcess(), place, sizeof(T));
		}

		template <typename T> static void Xor(DWORD place, T value)
		{
			return Xor<T>((void*)place, value);
		}

		template <typename T> static void Or(void* place, T value)
		{
			*(T*)place |= value;
			FlushInstructionCache(GetCurrentProcess(), place, sizeof(T));
		}

		template <typename T> static void Or(DWORD place, T value)
		{
			return Or<T>((void*)place, value);
		}

		template <typename T> static void And(void* place, T value)
		{
			*(T*)place &= value;
			FlushInstructionCache(GetCurrentProcess(), place, sizeof(T));
		}

		template <typename T> static void And(DWORD place, T value)
		{
			return And<T>((void*)place, value);
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
		void* Original;
		char Buffer[5];
		bool UseJump;

		DWORD Protection;

		std::mutex StateMutex;
	};
}
