#pragma once

#define HOOK_JUMP true
#define HOOK_CALL false

namespace Utils
{
  // Function type, such as void (int), that a game function has.
  //
  template <typename T>
  concept Function = std::is_function_v<T>;

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
      Signature(uintptr_t _start, size_t _length) : Signature(reinterpret_cast<void*>(_start), _length) {}

      void process();
      void add(const Container& container);

    private:
      void* start;
      size_t length;
      std::vector<Container> signatures;
    };

    Hook() : initialized(false), installed(false), place(nullptr), stub(nullptr), useJump(false), protection(0) { ZeroMemory(this->buffer, sizeof(this->buffer)); }

    Hook(void* place, void* stub, bool useJump = true) : Hook() { this->initialize(place, stub, useJump); }
    Hook(void* place, void(*stub)(), bool useJump = true) : Hook(place, reinterpret_cast<void*>(stub), useJump) {}

    Hook(uintptr_t place, void* stub, bool useJump = true) : Hook(reinterpret_cast<void*>(place), stub, useJump) {}
    Hook(uintptr_t place, uintptr_t stub, bool useJump = true) : Hook(reinterpret_cast<void*>(place), reinterpret_cast<void*>(stub), useJump) {}
    Hook(uintptr_t place, void(*stub)(), bool useJump = true) : Hook(reinterpret_cast<void*>(place), reinterpret_cast<void*>(stub), useJump) {}

    template <typename Stub> requires std::is_function_v<Stub>
    Hook(void* place, Stub* stub, bool useJump = true)
      : Hook(place, reinterpret_cast<void*>(stub), useJump) {}

    template <typename Stub> requires std::is_function_v<Stub>
    Hook(uintptr_t place, Stub* stub, bool useJump = true)
      : Hook(reinterpret_cast<void*>(place), stub, useJump) {}

    template <typename Target, typename Stub> requires std::is_function_v<Target>
    Hook(Target* place, Stub stub, bool useJump = true)
      : Hook(reinterpret_cast<void*>(place), stub, useJump) {}

    ~Hook();

    Hook* initialize(void* place, void* stub, bool useJump = true);
    Hook* initialize(uintptr_t place, void* stub, bool useJump = true);
    Hook* initialize(uintptr_t place, void(*stub)(), bool useJump = true); // For lambdas
    Hook* install(bool unprotect = true, bool keepUnprotected = false);
    Hook* uninstall(bool unprotect = true);

    void* getAddress();
    void quick();

    // Return the game function at the address as a pointer of the function
    // type T.
    //
    // Note that we return a plain function pointer. Wrapped in std::function,
    // a call to a constant address gets inlined by GCC down to a call to an
    // integer, which it then replaces with a trap since the integer isn't a
    // function.
    //
    template <Function T> static T* Call(uintptr_t function)
    {
      return reinterpret_cast<T*>(function);
    }

    template <Function T> static T* Call(FARPROC function)
    {
      return Call<T>(reinterpret_cast<uintptr_t>(function));
    }

    template <Function T> static T* Call(void* function)
    {
      return Call<T>(reinterpret_cast<uintptr_t>(function));
    }

    static void SetString(void* place, const char* string, size_t length);
    static void SetString(uintptr_t place, const char* string, size_t length);

    static void SetString(void* place, const char* string);
    static void SetString(uintptr_t place, const char* string);

    static void Nop(void* place, size_t length);
    static void Nop(uintptr_t place, size_t length);

    static void RedirectJump(void* place, void* stub);
    static void RedirectJump(uintptr_t place, void* stub);

    // Allocate size bytes of zeroed, read-write memory that a 32-bit
    // displacement from place reaches throughout. Throw std::runtime_error if
    // there is no free spot in range.
    //
    // Note that the game reaches its globals through such displacements,
    // either from the next instruction or from the image base it keeps in a
    // register, so this is where a global it outgrows has to move.
    //
    static void* AllocateNear(uintptr_t place, size_t size);

    // Send the function at place to stub and return a trampoline that runs
    // the function as it was. Throw std::runtime_error if length is too short
    // for a jump.
    //
    // Note that the first length bytes at place must be whole instructions
    // that don't depend on where they are, since the trampoline runs them
    // elsewhere before jumping back past them.
    //
    static void* Detour(uintptr_t place, size_t length, void* stub);

    template <Function T>
    static T* Detour(uintptr_t place, size_t length, T* stub)
    {
      return reinterpret_cast<T*>(Detour(place, length, reinterpret_cast<void*>(stub)));
    }

    // Replace the 32-bit value from with to in the instruction at place of
    // length bytes, be it an immediate or a displacement. Throw
    // std::runtime_error unless exactly one field holds from.
    //
    static void Replace32(uintptr_t place, size_t length, uint32_t from, uint32_t to);

    // Make the instruction at place of length bytes reach to instead of
    // from through its rip-relative operand.
    //
    static void Displace(uintptr_t place, size_t length, uintptr_t from, uintptr_t to);

    // Make the instruction at place of length bytes reach to instead of
    // from through its displacement from base, such as the image base the
    // game keeps in a register.
    //
    static void Rebase(uintptr_t place, size_t length, uintptr_t base, uintptr_t from, uintptr_t to);

    template <typename T> static void Set(void* place, T value)
    {
      DWORD oldProtect;
      VirtualProtect(place, sizeof(T), PAGE_EXECUTE_READWRITE, &oldProtect);

      *static_cast<T*>(place) = value;

      VirtualProtect(place, sizeof(T), oldProtect, &oldProtect);
      FlushInstructionCache(GetCurrentProcess(), place, sizeof(T));
    }

    template <typename T> static void Set(uintptr_t place, T value)
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

    template <typename T> static void Xor(uintptr_t place, T value)
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

    template <typename T> static void Or(uintptr_t place, T value)
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

    template <typename T> static void And(uintptr_t place, T value)
    {
      return And<T>(reinterpret_cast<void*>(place), value);
    }

    template <typename T> static T Get(void* place)
    {
      return *static_cast<T*>(place);
    }

    template <typename T> static T Get(uintptr_t place)
    {
      return Get<T>(reinterpret_cast<void*>(place));
    }

  private:
    bool initialized;
    bool installed;

    void* place;
    void* stub;
    char buffer[5];
    bool useJump;

    DWORD protection;

    std::mutex stateMutex;
  };
}
