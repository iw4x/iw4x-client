#pragma once

#define HOOK_JUMP true
#define HOOK_CALL false

namespace Utils
{
	class Hook
	{
	public:
		class Signature
		{
		public:
			struct Container
			{
				const char* signature;
				const char* mask;
				std::function<void(char*)> callback;
			};

			Signature(void* _start, size_t _length) : start(_start), length(_length) {}
			Signature(DWORD _start, size_t _length) : Signature(reinterpret_cast<void*>(_start), _length) {}
			Signature() : Signature(0x400000, 0x800000) {}

			void process();
			void add(Container& container);

		private:
			void* start;
			size_t length;
			std::vector<Container> signatures;
		};

		class Interceptor
		{
		public:
			static void Install(void* place, void* stub);
			static void Install(void* place, void(*stub)());
			static void Install(void** place, void(*stub)());

		private:
			static std::map<void*, void*> IReturn;
			static std::map<void*, void(*)()> ICallbacks;

			static void InterceptionStub();
			static void RunCallback(void* place);
			static void* PopReturn(void* place);
		};

		Hook() : initialized(false), installed(false), place(nullptr), stub(nullptr), original(nullptr), useJump(false), protection(0) { ZeroMemory(this->buffer, sizeof(this->buffer)); }

		Hook(void* place, void* stub, bool useJump = true) : Hook() { this->initialize(place, stub, useJump); }
		Hook(void* place, void(*stub)(), bool useJump = true) : Hook(place, reinterpret_cast<void*>(stub), useJump) {}

		Hook(DWORD place, void* stub, bool useJump = true) : Hook(reinterpret_cast<void*>(place), stub, useJump) {}
		Hook(DWORD place, DWORD stub, bool useJump = true) : Hook(reinterpret_cast<void*>(place), reinterpret_cast<void*>(stub), useJump) {}
		Hook(DWORD place, void(*stub)(), bool useJump = true) : Hook(reinterpret_cast<void*>(place), reinterpret_cast<void*>(stub), useJump) {}

		~Hook();

		Hook* initialize(void* place, void* stub, bool useJump = true);
		Hook* initialize(DWORD place, void* stub, bool useJump = true);
		Hook* initialize(DWORD place, void(*stub)(), bool useJump = true); // For lambdas
		Hook* install(bool unprotect = true, bool keepUnprotected = false);
		Hook* uninstall(bool unprotect = true);

		void* getAddress();
		void quick();

		template <typename T> static std::function<T> Call(DWORD function)
		{
			return std::function<T>(reinterpret_cast<T*>(function));
		}

		template <typename T> static std::function<T> Call(FARPROC function)
		{
			return Call<T>(reinterpret_cast<DWORD>(function));
		}

		template <typename T> static std::function<T> Call(void* function)
		{
			return Call<T>(reinterpret_cast<DWORD>(function));
		}

		static void SetString(void* place, const char* string, size_t length);
		static void SetString(DWORD place, const char* string, size_t length);

		static void SetString(void* place, const char* string);
		static void SetString(DWORD place, const char* string);

		static void Nop(void* place, size_t length);
		static void Nop(DWORD place, size_t length);

		static void RedirectJump(void* place, void* stub);
		static void RedirectJump(DWORD place, void* stub);

		template <typename T> static void Set(void* place, T value)
		{
			DWORD oldProtect;
			VirtualProtect(place, sizeof(T), PAGE_EXECUTE_READWRITE, &oldProtect);

			*static_cast<T*>(place) = value;

			VirtualProtect(place, sizeof(T), oldProtect, &oldProtect);
			FlushInstructionCache(GetCurrentProcess(), place, sizeof(T));
		}

		template <typename T> static void Set(DWORD place, T value)
		{
			return Set<T>(reinterpret_cast<void*>(place), value);
		}

		template <typename T> static void Xor(void* place, T value)
		{
			DWORD oldProtect;
			VirtualProtect(place, sizeof(T), PAGE_EXECUTE_READWRITE, &oldProtect);

			*static_cast<T*>(place) ^= value;

			VirtualProtect(place, sizeof(T), oldProtect, &oldProtect);
			FlushInstructionCache(GetCurrentProcess(), place, sizeof(T));
		}

		template <typename T> static void Xor(DWORD place, T value)
		{
			return Xor<T>(reinterpret_cast<void*>(place), value);
		}

		template <typename T> static void Or(void* place, T value)
		{
			DWORD oldProtect;
			VirtualProtect(place, sizeof(T), PAGE_EXECUTE_READWRITE, &oldProtect);

			*static_cast<T*>(place) |= value;

			VirtualProtect(place, sizeof(T), oldProtect, &oldProtect);
			FlushInstructionCache(GetCurrentProcess(), place, sizeof(T));
		}

		template <typename T> static void Or(DWORD place, T value)
		{
			return Or<T>(reinterpret_cast<void*>(place), value);
		}

		template <typename T> static void And(void* place, T value)
		{
			DWORD oldProtect;
			VirtualProtect(place, sizeof(T), PAGE_EXECUTE_READWRITE, &oldProtect);

			*static_cast<T*>(place) &= value;

			VirtualProtect(place, sizeof(T), oldProtect, &oldProtect);
			FlushInstructionCache(GetCurrentProcess(), place, sizeof(T));
		}

		template <typename T> static void And(DWORD place, T value)
		{
			return And<T>(reinterpret_cast<void*>(place), value);
		}

		template <typename T> static T Get(void* place)
		{
			return *static_cast<T*>(place);
		}

		template <typename T> static T Get(DWORD place)
		{
			return Get<T>(reinterpret_cast<void*>(place));
		}

	private:
		bool initialized;
		bool installed;

		void* place;
		void* stub;
		void* original;
		char buffer[5];
		bool useJump;

		DWORD protection;

		std::mutex stateMutex;
	};
}
