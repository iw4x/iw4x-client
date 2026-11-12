#include "Hooking.hpp"

namespace Utils
{
  namespace
  {
    // Relay that jumps to a target a rel32 branch can't reach:
    //
    //    jmp qword ptr [rip]
    //    dq  <target>
    //
    // Padded with int3 to 16 bytes.
    //
    constexpr std::size_t RelaySize = 16;

    // Page of relays and the number of bytes taken in it.
    //
    struct RelayPage
    {
      uint8_t* base;
      std::size_t size;
      std::size_t used;
    };

    std::mutex RelayMutex;
    std::vector<RelayPage> RelayPages;

    // Return true if a rel32 branch whose next instruction is at next
    // reaches target.
    //
    bool Reaches(uintptr_t next, uintptr_t target)
    {
      const intptr_t d = static_cast<intptr_t>(target - next);
      return d >= std::numeric_limits<int32_t>::min() && d <= std::numeric_limits<int32_t>::max();
    }

    // Allocate a relay page within rel32 reach of next. Throw
    // std::runtime_error if there is no free spot in range.
    //
    RelayPage AllocateRelayPage(uintptr_t next)
    {
      SYSTEM_INFO info;
      GetSystemInfo(&info);

      const uintptr_t granularity = info.dwAllocationGranularity;
      const std::size_t size = info.dwPageSize;

      auto tryAllocate = [size, next] (uintptr_t a) -> uint8_t*
      {
        if (!Reaches(next, a) || !Reaches(next, a + size))
        {
          return nullptr;
        }

        MEMORY_BASIC_INFORMATION mbi;
        if (VirtualQuery(reinterpret_cast<void*>(a), &mbi, sizeof(mbi)) == 0 || mbi.State != MEM_FREE)
        {
          return nullptr;
        }

        return static_cast<uint8_t*>(VirtualAlloc(reinterpret_cast<void*>(a), size, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READ));
      };

      const uintptr_t start = next & ~(granularity - 1);
      const uintptr_t span = 0x80000000;

      for (uintptr_t a = start; a >= granularity && start - a < span; a -= granularity)
      {
        if (uint8_t* p = tryAllocate(a))
        {
          return {p, size, 0};
        }
      }

      for (uintptr_t a = start + granularity; a - start < span; a += granularity)
      {
        if (uint8_t* p = tryAllocate(a))
        {
          return {p, size, 0};
        }
      }

      throw std::runtime_error(std::format("unable to allocate hook relay near {:#x}", next));
    }

    // Return target or, if a rel32 branch whose next instruction is at next
    // doesn't reach it, a relay to it.
    //
    void* Reach(void* next, void* target)
    {
      const uintptr_t n = reinterpret_cast<uintptr_t>(next);

      if (Reaches(n, reinterpret_cast<uintptr_t>(target)))
      {
        return target;
      }

      std::scoped_lock lock(RelayMutex);

      // Find a page in range with room for another relay, allocating one if
      // there is none.
      //
      RelayPage* page = nullptr;
      for (RelayPage& p : RelayPages)
      {
        const uintptr_t b = reinterpret_cast<uintptr_t>(p.base);

        if (p.used + RelaySize <= p.size && Reaches(n, b) && Reaches(n, b + p.size))
        {
          page = &p;
          break;
        }
      }

      if (page == nullptr)
      {
        page = &RelayPages.emplace_back(AllocateRelayPage(n));
      }

      // Write the relay. Note that we keep the page executable while we
      // write since other threads may be running its earlier relays.
      //
      uint8_t* r = page->base + page->used;

      DWORD protection;
      VirtualProtect(r, RelaySize, PAGE_EXECUTE_READWRITE, &protection);

      const uint8_t jump[] = {0xFF, 0x25, 0x00, 0x00, 0x00, 0x00};
      std::memcpy(r, jump, sizeof(jump));
      std::memcpy(r + sizeof(jump), &target, sizeof(target));
      std::memset(r + sizeof(jump) + sizeof(target), 0xCC, RelaySize - sizeof(jump) - sizeof(target));

      VirtualProtect(r, RelaySize, protection, &protection);
      FlushInstructionCache(GetCurrentProcess(), r, RelaySize);

      page->used += RelaySize;
      return r;
    }
  }

  void Hook::Signature::process()
  {
    if (this->signatures.empty()) return;

    char* _start = static_cast<char*>(this->start);

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
          if (container->mask[j] != '?' && container->signature[j] != address[j])
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

  void Hook::Signature::add(const Hook::Signature::Container& container)
  {
    Hook::Signature::signatures.push_back(container);
  }

  Hook::~Hook()
  {
    if (this->initialized)
    {
      this->uninstall();
    }
  }

  Hook* Hook::initialize(uintptr_t _place, void(*_stub)(), bool _useJump)
  {
    return this->initialize(_place, reinterpret_cast<void*>(_stub), _useJump);
  }

  Hook* Hook::initialize(uintptr_t _place, void* _stub, bool _useJump)
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

    void* target = Reach(code + 5, this->stub);
    *reinterpret_cast<int32_t*>(code + 1) = static_cast<int32_t>(reinterpret_cast<intptr_t>(target) - reinterpret_cast<intptr_t>(code + 5));

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

  void Hook::Nop(uintptr_t place, size_t length)
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

  void Hook::SetString(uintptr_t place, const char* string, size_t length)
  {
    Hook::SetString(reinterpret_cast<void*>(place), string, length);
  }

  void Hook::SetString(void* place, const char* string)
  {
    Hook::SetString(place, string, strlen(static_cast<char*>(place)));
  }

  void Hook::SetString(uintptr_t place, const char* string)
  {
    Hook::SetString(reinterpret_cast<void*>(place), string);
  }

  void Hook::RedirectJump(void* place, void* stub)
  {
    char* operandPtr = static_cast<char*>(place) + 2;
    void* target = Reach(static_cast<char*>(place) + 6, stub);
    int32_t newOperand = static_cast<int32_t>(reinterpret_cast<intptr_t>(target) - reinterpret_cast<intptr_t>(static_cast<char*>(place) + 6));
    Utils::Hook::Set<int32_t>(operandPtr, newOperand);
  }

  void Hook::RedirectJump(uintptr_t place, void* stub)
  {
    Hook::RedirectJump(reinterpret_cast<void*>(place), stub);
  }
}
