#include "WineD3D.hpp"

// WineD3D has a 128 MiB limit on how much buffer memory it keeps mapped
// in a 32-bit process. The problem is that it allocates this memory in
// 64 MiB chunks, so just two chunks (one containing IW4's dynamic
// vertex and index buffers) are enough to exhaust the limit.
//
// Things get worse during map loading, where IW4 keeps some additional
// buffers mapped (about 24 MiB on mp_rust). This pushes us over the
// limit, causing WineD3D to unmap the dynamic buffers after every
// unlock. Each subsequent lock then has to wait for the GPU and, with a
// few hundred of these per frame, we can end up at around 5 FPS.
// Interestingly, this usually goes away once the player starts moving
// and the extra mappings are released.
//
// The workaround is to increase the limit in wined3d_bo_gl_unmap(),
// which turns out to be a single comparison against 0x08000000 (128
// MiB). We patch its immediate operand to 512 MiB, which should leave
// enough room for the game's additional mappings.
//
// @@: 512 MiB may not be sufficient for heavily modded maps. Perhaps we
// should consider removing the limit altogether.
//
namespace Components
{
  namespace
  {
    // Note that IW4x is large-address-aware, so we have enough address
    // space for the extra mappings.
    //
    constexpr std::uint32_t MappingBudget = 512 * 1024 * 1024;

    // The comparison in wined3d_bo_gl_unmap() looks like this:
    //
    //   cmp dword ptr [edx + mapped_size], 8000000h
    //   jle ...
    //
    // Unfortunately, the mapped_size offset changes between Wine versions.
    // So we leave its lower 16 bits unmatched and assume the upper 16 bits
    // are zero. We also match the following conditional jump to narrow down
    // the search.
    //
    constexpr std::array<std::uint8_t, 12> BudgetPattern =
    {
      0x81, 0xBA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x0F, 0x8E
    };

    constexpr std::array<bool, 12> BudgetMask =
    {
      true, true, false, false, true, true, true, true, true, true, true, true
    };

    constexpr std::size_t BudgetImmediate = 6;
  }

  // Search the executable sections for the budget comparison and patch it
  // if there is exactly one match. Return the number of matches so the
  // caller can report when the patch could not be applied.
  //
  // @@: Perhaps we could use Utils::Hook::Signature here.
  //
  std::size_t WineD3D::RaiseMappingBudget(unsigned char* module)
  {
    const auto* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(module);
    const auto* nt = reinterpret_cast<const IMAGE_NT_HEADERS*>(module + dos->e_lfanew);
    const auto* section = IMAGE_FIRST_SECTION(nt);

    std::size_t matches = 0;
    unsigned char* found = nullptr;

    for (WORD i = 0; i < nt->FileHeader.NumberOfSections; ++i, ++section)
    {
      if (!(section->Characteristics & IMAGE_SCN_MEM_EXECUTE))
      {
        continue;
      }

      unsigned char* begin = module + section->VirtualAddress;
      const std::size_t size = section->Misc.VirtualSize;

      for (std::size_t j = 0; j + BudgetPattern.size() <= size; ++j)
      {
        bool match = true;

        for (std::size_t k = 0; k < BudgetPattern.size() && match; ++k)
        {
          match = !BudgetMask[k] || begin[j + k] == BudgetPattern[k];
        }

        if (match)
        {
          ++matches;
          found = begin + j;
        }
      }
    }

    if (matches == 1)
    {
      Utils::Hook::Set<std::uint32_t>(found + BudgetImmediate, MappingBudget);
    }

    return matches;
  }

  WineD3D::WineD3D()
  {
    if (Dedicated::IsEnabled() || !Utils::IsWineEnvironment())
    {
      return;
    }

    // Normally wined3d.dll should already be loaded through d3d9.dll at
    // this point. If it's missing, then we are presumably using another
    // Direct3D implementation (such as DXVK) and have nothing to patch.
    //
    auto* module = reinterpret_cast<unsigned char*>(GetModuleHandleA("wined3d.dll"));
    if (!module)
      return;

    const std::size_t matches = RaiseMappingBudget(module);

    // We cannot use the console yet, so defer reporting until the game
    // has finished initializing.
    //
    Scheduler::Once([matches]
    {
      if (matches == 1)
      {
        Logger::Print("Raised the wined3d mapping budget to {}MB\n", MappingBudget >> 20);
      }
      else
      {
        Logger::Warning(Game::CON_CHANNEL_GFX, "Couldn't find the wined3d mapping budget ({} matches), dynamic buffers may stall\n", matches);
      }
    }, Scheduler::Pipeline::MAIN);
  }
}
