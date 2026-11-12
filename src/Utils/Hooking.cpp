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

    // Allocate size bytes with protection, rounded up to whole pages, such
    // that a 32-bit displacement from place reaches each of them. Throw
    // std::runtime_error if there is no free spot in range.
    //
    uint8_t* AllocateReachable(uintptr_t place, std::size_t size, DWORD protection)
    {
      SYSTEM_INFO info;
      GetSystemInfo(&info);

      const uintptr_t granularity = info.dwAllocationGranularity;
      const std::size_t page = info.dwPageSize;

      size = (size + page - 1) / page * page;

      auto tryAllocate = [place, size, protection] (uintptr_t a) -> uint8_t*
      {
        if (!Reaches(place, a) || !Reaches(place, a + size))
        {
          return nullptr;
        }

        MEMORY_BASIC_INFORMATION mbi;
        if (VirtualQuery(reinterpret_cast<void*>(a), &mbi, sizeof(mbi)) == 0 ||
            mbi.State != MEM_FREE                                            ||
            reinterpret_cast<uintptr_t>(mbi.BaseAddress) + mbi.RegionSize - a < size)
        {
          return nullptr;
        }

        return static_cast<uint8_t*>(VirtualAlloc(reinterpret_cast<void*>(a), size, MEM_RESERVE | MEM_COMMIT, protection));
      };

      const uintptr_t start = place & ~(granularity - 1);
      const uintptr_t span = 0x80000000;

      for (uintptr_t a = start; a >= granularity && start - a < span; a -= granularity)
      {
        if (uint8_t* p = tryAllocate(a))
        {
          return p;
        }
      }

      for (uintptr_t a = start + granularity; a - start < span; a += granularity)
      {
        if (uint8_t* p = tryAllocate(a))
        {
          return p;
        }
      }

      throw std::runtime_error(std::format("unable to allocate {} bytes near {:#x}", size, place));
    }

    // Allocate a relay page within rel32 reach of next. Throw
    // std::runtime_error if there is no free spot in range.
    //
    RelayPage AllocateRelayPage(uintptr_t next)
    {
      SYSTEM_INFO info;
      GetSystemInfo(&info);

      return {AllocateReachable(next, info.dwPageSize, PAGE_EXECUTE_READ), info.dwPageSize, 0};
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

    std::size_t sigCount = this->signatures.size();
    Hook::Signature::Container* containers = this->signatures.data();

    for (size_t i = 0; i < this->length; ++i)
    {
      char* address = _start + i;

      for (std::size_t k = 0; k < sigCount; ++k)
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
  void* Hook::AllocateNear(uintptr_t place, size_t size)
  {
    return AllocateReachable(place, size, PAGE_READWRITE);
  }

  void Hook::Replace32(uintptr_t place, size_t length, uint32_t from, uint32_t to)
  {
    // Find the field among the bytes past the first opcode byte, insisting
    // on a single match so that we never patch an unrelated operand that
    // happens to hold the same value.
    //
    const uint8_t* code = reinterpret_cast<const uint8_t*>(place);
    size_t field = 0;
    size_t matches = 0;

    for (size_t i = 1; i + sizeof(uint32_t) <= length; ++i)
    {
      uint32_t v;
      std::memcpy(&v, code + i, sizeof(v));

      if (v == from)
      {
        field = i;
        ++matches;
      }
    }

    if (matches != 1)
    {
      throw std::runtime_error(std::format("{} matches of {:#x} in instruction at {:#x}", matches, from, place));
    }

    Set<uint32_t>(place + field, to);
  }

  void Hook::Displace(uintptr_t place, size_t length, uintptr_t from, uintptr_t to)
  {
    Rebase(place, length, place + length, from, to);
  }

  void Hook::Rebase(uintptr_t place, size_t length, uintptr_t base, uintptr_t from, uintptr_t to)
  {
    if (!Reaches(base, from) || !Reaches(base, to))
    {
      throw std::runtime_error(std::format("{:#x} out of 32-bit reach of {:#x} in instruction at {:#x}", Reaches(base, from) ? to : from, base, place));
    }

    Replace32(place, length, static_cast<uint32_t>(from - base), static_cast<uint32_t>(to - base));
  }
}
