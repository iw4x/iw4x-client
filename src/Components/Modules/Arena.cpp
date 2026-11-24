#include "Arena.hpp"

#include <algorithm> // ranges::find_if()
#include <cassert>
#include <cstddef>   // offsetof()
#include <format>    // format()
#include <limits>    // numeric_limits
#include <stdexcept> // length_error, runtime_error
#include <utility>   // move(), pair

using namespace std;

namespace Components
{
  namespace
  {
    // IW4 reads the arenas and builds the map list into global arrays whose
    // sizes are fixed at compile time. UI_LoadArenasFromFile() copies the info
    // string of each arena into a 64KB pool and keeps a pointer to it in
    // ui_arenaInfos, which has room for 64 arenas, and whatever doesn't fit is
    // dropped without a word. UI_LoadArenas() then makes an element of
    // sharedUiInfo.mapList for each arena (room for 128) and the menus sort
    // these by display name into sharedUiInfo.mapIndexSorted. The element also
    // has only 16 bytes for the name that the map is loaded by, so a map with
    // a longer name ends up being loaded by a truncated one, which presumably
    // fails. The arena file in question is mp/basemaps.arena or, with a user
    // map loaded, the one that comes with the map (see Maps).
    //
    // These limits look like they came from Quake III, whose UI kept the arena
    // strings in a fixed pool as well, and nothing else in IW4 seems to care
    // about them. So what can we do? We could reimplement UI_LoadArenas() and
    // keep the maps wherever we like, but then every function that reads
    // mapList would have to follow, and there are quite a few of them (the
    // menu feeders and the lookups by name, for example). We could also make
    // the arrays bigger and call it a day, which is more or less what IW4x
    // used to do, but that just moves the limit somewhere else. So what we do
    // instead is move the four arrays into one block of memory that we
    // allocate and make the block bigger whenever the arena file turns out to
    // have more arenas than there is room for. And while at it, we give each
    // map list element a longer name at its end (see newMapArena_t in
    // Structs.hpp).
    //
    // Moving an array is where it gets hairy. IW4 reaches its globals through
    // 32-bit displacements, either from the next instruction or from
    // sharedUiInfo, whose address it keeps in a register in some of its
    // functions. So the block has to be within 2GB of IW4's image and every
    // instruction that reaches a moved array has to be rewritten, along with
    // the ones that hold its size. We describe each array, together with the
    // instructions that depend on it, in global_info and let global do the
    // rewriting (arena_globals then moves the four of them in step).
    //
    // Note that UI_LoadArenas() seems to be inlined into UI_UpdateArenas(),
    // which is why Functions.cpp has both names for the one address.
    //
    constexpr uintptr_t shared_ui_info (0x1465CE8C0);

    // We start with room for 128 arenas, which is what IW4x had before and
    // probably more than a typical install has, so normally the arena file is
    // only read once.
    //
    constexpr int initial_arenas (128);

    // How an instruction reaches a global (see above for the two cases).
    //
    enum class base {rip, shared_ui_info};

    // An instruction that reaches a global. The offset is where in the
    // global's first element the displacement points, which is normally the
    // start of the element or one of its fields (and, in one case, just before
    // it).
    //
    struct reference
    {
      uintptr_t place;
      size_t    length;
      base      via;
      ptrdiff_t offset;

      // Return the address that the displacement counts from.
      //
      uintptr_t
      base_address () const noexcept
      {
        return via == base::rip ? place + length : shared_ui_info;
      }
    };

    // An instruction that holds, as an immediate value, the number of elements
    // that a global has room for.
    //
    struct limit
    {
      uintptr_t place;
      size_t    length;
    };

    // An instruction that holds something that depends on the element layout,
    // normally the element size, with its value for IW4's elements and for
    // ours.
    //
    struct constant
    {
      uintptr_t place;
      size_t    length;
      ptrdiff_t game;
      ptrdiff_t ours;
    };

    // A global array as IW4 has it, together with the instructions that depend
    // on where it is and on how big it is.
    //
    // Note that size is the size of our element, which only differs from IW4's
    // for the map list. In that case constants lists the instructions that
    // have to change along with the element, and field maps the offset of a
    // field in IW4's element to where the field is in ours (NULL if the two
    // layouts are the same).
    //
    struct global_info
    {
      const char* name;
      uintptr_t   address;
      int         capacity;
      size_t      size;

      span<const reference> references;
      span<const limit>     limits;
      span<const constant>  constants;

      ptrdiff_t (*field) (ptrdiff_t offset);
    };

    // The pointers to the info strings, one per arena.
    //
    const reference ui_arena_infos_references[]
    {
      {0x140269A1B, 7, base::rip, 0},
      {0x140269D78, 7, base::rip, 0}
    };

    // UI_LoadArenasFromFile() subtracts the arenas it already has from this
    // and stops reading once it has taken that many. It doesn't say anything
    // when that happens, which load_arenas_from_file() has to work around.
    //
    const limit ui_arena_infos_limits[]
    {
      {0x140269A32, 5} // mov ebx, 40h
    };

    const global_info ui_arena_infos
    {
      "ui_arenaInfos",
      0x1465BE5C0,
      64,
      sizeof (char*),
      ui_arena_infos_references,
      ui_arena_infos_limits,
      {},     /* constants */
      nullptr /* field */
    };

    // The most that one arena can take in the memory pool.
    //
    // For each arena UI_LoadArenasFromFile() takes the length of its info
    // string plus 6 bytes plus the number of digits in 64. This looks like
    // Quake III's "extra space for arena number", that is, the terminator and
    // room to add a "\num\" key, which nobody seems to add any more. Note that
    // the 64 there is a constant rather than the count, which is why we leave
    // it alone. As for the info string, it is built with
    // Info_SetValueForKey(), which won't take it past 1024 characters.
    //
    constexpr int arena_text_size (1024 + 8);

    // The pool that the info strings are copied to. IW4 doesn't seem to have a
    // name for it, so we borrow memoryPool from Quake III.
    //
    // If the pool runs out, then IW4 drops with "ARENA_BUFFER_SIZE exceeded".
    // So we would rather size it for the worst case and waste some memory than
    // try to guess what a typical arena takes.
    //
    const reference memory_pool_references[]
    {
      {0x140269A75, 7, base::rip, 0}
    };

    const limit memory_pool_limits[]
    {
      {0x140269B6D, 6} // cmp ecx, 10000h
    };

    const global_info memory_pool
    {
      "memoryPool",
      0x1465BE7C0,
      0x10000,
      sizeof (char),
      memory_pool_references,
      memory_pool_limits,
      {},     /* constants */
      nullptr /* field */
    };

    // The fields of a map list element that IW4 reaches directly, plus the
    // longer name at the end of ours.
    //
    constexpr ptrdiff_t map_load_name    (offsetof (Game::mapInfo, mapLoadName));
    constexpr ptrdiff_t map_description  (offsetof (Game::mapInfo, mapDescription));
    constexpr ptrdiff_t map_load_image   (offsetof (Game::mapInfo, mapLoadImage));
    constexpr ptrdiff_t map_custom_count (offsetof (Game::mapInfo, mapCustomCount));
    constexpr ptrdiff_t type_bits        (offsetof (Game::mapInfo, typeBits));
    constexpr ptrdiff_t active           (offsetof (Game::mapInfo, active));
    constexpr ptrdiff_t map_name         (offsetof (Game::newMapArena_t, mapName));

    constexpr ptrdiff_t map_info_size      (sizeof (Game::mapInfo));
    constexpr ptrdiff_t new_map_arena_size (sizeof (Game::newMapArena_t));

    const reference map_list_references[]
    {
      {0x140269912,  7, base::rip,            map_load_name},
      {0x140269989,  7, base::rip,            0},
      {0x140269C46,  7, base::rip,            0},
      {0x140269D6C,  7, base::rip,            0},
      {0x140269D84,  7, base::rip,            map_load_name},
      {0x140269E1F,  7, base::rip,            map_description},
      {0x140269E54,  7, base::rip,            map_load_image},
      {0x14026A08B,  7, base::rip,            map_load_name},
      {0x14026A096,  7, base::rip,            0},
      {0x14026D05A,  7, base::rip,            map_custom_count},
      {0x14026D1BE,  7, base::rip,            0},
      {0x14026D786,  7, base::rip,            0},
      {0x14026E15B,  7, base::rip,            active},

      {0x140269EA5,  8, base::shared_ui_info, type_bits},
      {0x140269F24,  8, base::shared_ui_info, type_bits},
      {0x140269F2F,  8, base::shared_ui_info, type_bits},
      {0x140269F68, 12, base::shared_ui_info, type_bits},
      {0x140269F7B,  7, base::shared_ui_info, 0},
      {0x14026D137,  7, base::shared_ui_info, map_load_name},
      {0x14026D17E,  7, base::shared_ui_info, map_load_image}
    };

    // UI_LoadArenas() stops making elements once the count gets to this limit.
    // And since the menus index mapIndexSorted with the same count, it covers
    // that array as well.
    //
    const limit map_list_limits[]
    {
      {0x14026A082, 5} // cmp eax, 80h
    };

    // The instructions that step from one element to the next and so multiply
    // by the element size. Then there is the odd one out, the 'lea rdx,
    // [rdi-0A50h]' in the lookup of a map's custom value, which gets from
    // mapCustomCount back to the map name.
    //
    const constant map_list_constants[]
    {
      {0x140269926, 7, map_info_size, new_map_arena_size},
      {0x140269990, 7, map_info_size, new_map_arena_size},
      {0x140269C53, 7, map_info_size, new_map_arena_size},
      {0x140269C88, 7, map_info_size, new_map_arena_size},
      {0x140269DC9, 7, map_info_size, new_map_arena_size},
      {0x140269DF7, 7, map_info_size, new_map_arena_size},
      {0x140269E26, 7, map_info_size, new_map_arena_size},
      {0x140269E5B, 7, map_info_size, new_map_arena_size},
      {0x140269E9E, 7, map_info_size, new_map_arena_size},
      {0x140269EB4, 7, map_info_size, new_map_arena_size},
      {0x140269F1D, 7, map_info_size, new_map_arena_size},
      {0x140269F61, 7, map_info_size, new_map_arena_size},
      {0x140269F85, 7, map_info_size, new_map_arena_size},
      {0x14026D0B2, 7, map_info_size, new_map_arena_size},
      {0x14026D141, 7, map_info_size, new_map_arena_size},
      {0x14026D1CF, 7, map_info_size, new_map_arena_size},
      {0x14026D796, 7, map_info_size, new_map_arena_size},
      {0x14026E178, 7, map_info_size, new_map_arena_size},

      {0x14026D068,
       7,
       map_load_name - map_custom_count,
       map_name - map_custom_count}
    };

    // Make sure that sharedUiInfo_t in Structs.hpp agrees with the disassembly
    // on where the arrays are.
    //
    static_assert (shared_ui_info + offsetof (Game::sharedUiInfo_t, mapList) ==
                   0x1465D06D4);

    // Make whatever reaches mapLoadName reach mapName instead, which is how
    // IW4 ends up filling and reading our longer name. Note that the old name
    // field is still there, at the same place, and a few of the hooks below
    // rely on that (see set_map_name_dvar()).
    //
    ptrdiff_t
    map_list_field (ptrdiff_t offset)
    {
      return offset == map_load_name ? map_name : offset;
    }

    const global_info map_list
    {
      "sharedUiInfo.mapList",
      shared_ui_info + offsetof (Game::sharedUiInfo_t, mapList),
      extent_v<decltype (Game::sharedUiInfo_t::mapList)>,
      sizeof (Game::newMapArena_t),
      map_list_references,
      map_list_limits,
      map_list_constants,
      &map_list_field
    };

    // The position in mapList of each map, in the order that the menus show
    // them.
    //
    // Note that the sort also reaches mapIndexSorted[i - 1], which is the -4
    // below.
    //
    const reference map_index_sorted_references[]
    {
      {0x14026990B, 7, base::rip,             0},
      {0x14026997E, 7, base::rip,             0},

      {0x140269C64, 8, base::shared_ui_info,  0},
      {0x140269C80, 8, base::shared_ui_info,  0},
      {0x140269CC0, 8, base::shared_ui_info, -4},
      {0x140269CC8, 8, base::shared_ui_info,  0},
      {0x140269CD8, 8, base::shared_ui_info,  0}
    };

    static_assert (shared_ui_info +
                   offsetof (Game::sharedUiInfo_t, mapIndexSorted) ==
                   0x1466286D4);

    const global_info map_index_sorted
    {
      "sharedUiInfo.mapIndexSorted",
      shared_ui_info + offsetof (Game::sharedUiInfo_t, mapIndexSorted),
      extent_v<decltype (Game::sharedUiInfo_t::mapIndexSorted)>,
      sizeof (int),
      map_index_sorted_references,
      {},      /* limits (see map_list_limits) */
      {},      /* constants */
      nullptr  /* field */
    };

    // One of IW4's global arrays, wherever it happens to be at the moment.
    //
    // A global starts out where IW4 has it, with IW4's elements, and after the
    // first move its elements are ours. Either way we keep every instruction
    // in its info in agreement with where it is and how much room it has, so
    // that IW4 doesn't notice the difference (at least that's the idea).
    //
    class global
    {
    public:
      explicit
      global (const global_info& i) noexcept
        : info_ (i), address_ (i.address), capacity_ (i.capacity) {}

      const global_info&
      info () const noexcept {return info_;}

      int
      capacity () const noexcept {return capacity_;}

      // Return the size of n of our elements.
      //
      size_t
      size (int n) const noexcept {return n * info_.size;}

      // Return the first reference that can't reach this global at address or
      // NULL if they all can.
      //
      const reference*
      unreachable (uintptr_t address) const noexcept;

      // Move this global to address with room for capacity elements by
      // rewriting the instructions in its info. Note that nothing gets copied,
      // so the global starts out with whatever the caller has at address.
      //
      // Throw std::runtime_error if an instruction can't reach address, in
      // which case the instructions before it are left rewritten. So it's a
      // good idea to check unreachable() first.
      //
      void
      relocate (uintptr_t address, int capacity);

    private:
      // Return true if the global at address has our elements, which is the
      // case anywhere but where IW4 has it.
      //
      bool
      ours (uintptr_t address) const noexcept
      {
        return address != info_.address;
      }

      // Return the address that r reaches if the global is at address.
      //
      uintptr_t
      target (const reference& r, uintptr_t address) const noexcept;

      const global_info& info_;
      uintptr_t address_;
      int capacity_;
    };

    uintptr_t global::
    target (const reference& r, uintptr_t a) const noexcept
    {
      return a + (ours (a) && info_.field != nullptr
                  ? info_.field (r.offset)
                  : r.offset);
    }

    const reference* global::
    unreachable (uintptr_t a) const noexcept
    {
      auto i (ranges::find_if (info_.references, [this, a] (const reference& r)
      {
        return !Utils::Hook::Reaches (r.base_address (), target (r, a));
      }));

      return i != info_.references.end () ? &*i : nullptr;
    }

    void global::
    relocate (uintptr_t a, int n)
    {
      for (const reference& r: info_.references)
        Utils::Hook::Rebase (r.place,
                             r.length,
                             r.base_address (),
                             target (r, address_),
                             target (r, a));

      for (const limit& l: info_.limits)
        Utils::Hook::Replace32 (l.place, l.length, capacity_, n);

      // The constants only change along with the elements, which happens on
      // the first move (or would on a move back, but we never do that).
      //
      if (ours (a) != ours (address_))
      {
        for (const constant& c: info_.constants)
          Utils::Hook::Replace32 (c.place,
                                  c.length,
                                  ours (address_) ? c.ours : c.game,
                                  ours (a) ? c.ours : c.game);
      }

      address_ = a;
      capacity_ = n;
    }

    // Memory near IW4's image, as returned by AllocateNear(), that is freed on
    // destruction.
    //
    class auto_near
    {
    public:
      auto_near () noexcept: p_ (nullptr) {}

      explicit
      auto_near (void* p) noexcept: p_ (p) {}

      auto_near (auto_near&& x) noexcept: p_ (x.release ()) {}
      auto_near& operator= (auto_near&&) noexcept;

      auto_near (const auto_near&) = delete;
      auto_near& operator= (const auto_near&) = delete;

      ~auto_near () noexcept {reset ();}

      void*
      get () const noexcept {return p_;}

      void
      reset (void* p = nullptr) noexcept;

      void*
      release () noexcept
      {
        void* r (p_);
        p_ = nullptr;
        return r;
      }

    private:
      void* p_;
    };

    auto_near& auto_near::
    operator= (auto_near&& x) noexcept
    {
      reset (x.release ());
      return *this;
    }

    void auto_near::
    reset (void* p) noexcept
    {
      if (p_ != nullptr)
        Utils::Hook::FreeNear (p_);

      p_ = p;
    }

    // The four globals, which we keep together in one block with the same room
    // for arenas in each.
    //
    class arena_globals
    {
    public:
      arena_globals () noexcept
        : ui_arena_infos_ (ui_arena_infos),
          memory_pool_ (memory_pool),
          map_list_ (map_list),
          map_index_sorted_ (map_index_sorted) {}

      // Return the number of arenas that there is room for.
      //
      int
      capacity () const noexcept {return ui_arena_infos_.capacity ();}

      // Make sure there is room for at least n arenas, moving the globals to a
      // new block if there isn't. Throw std::length_error if IW4 can't count
      // that much text and std::runtime_error if the new block is out of
      // reach, in which case the globals are left where they were.
      //
      // Note that the globals come over empty since we don't copy anything.
      // This is fine as long as we only move them right before IW4 reads the
      // arena file, which is what both of our callers do.
      //
      // @@ We never shrink the block, say, when a user map replaces a long
      //    arena file with one arena. Maybe we should, but a few hundred KB
      //    doesn't seem worth another move.
      //
      void
      reserve (int n);

      // Return the map list (see arena::maps()).
      //
      span<const Game::newMapArena_t>
      maps () const noexcept;

      // Return the element whose mapLoadName (oldMapName in ours) is at
      // load_name.
      //
      const Game::newMapArena_t&
      map (const char* load_name) const noexcept;

    private:
      global ui_arena_infos_;
      global memory_pool_;
      global map_list_;
      global map_index_sorted_;

      auto_near block_;
    };

    void arena_globals::
    reserve (int n)
    {
      assert (n > 0);

      if (block_.get () != nullptr && n <= capacity ())
        return;

      // IW4 compares what it has used of the memory pool with the pool's size
      // as signed 32-bit values. The pool is also the biggest of the four, so
      // if it fits, then so do the rest.
      //
      if (n > numeric_limits<int>::max () / arena_text_size)
      {
        auto d (format ("memory pool for {} arenas exceeds {} bytes",
                        n,
                        numeric_limits<int>::max ()));

        throw length_error (std::move (d));
      }

      // Lay the globals out with the map list first, since that's where maps()
      // looks for it. Its elements happen to be a multiple of 8 bytes, so the
      // pointers that follow are aligned and nothing after them needs more
      // than that.
      //
      static_assert (sizeof (Game::newMapArena_t) % alignof (char*) == 0);

      const pair<global&, int> layout[]
      {
        {map_list_,         n},
        {ui_arena_infos_,   n},
        {map_index_sorted_, n},
        {memory_pool_,      n * arena_text_size}
      };

      size_t size (0);
      for (const auto& [g, c]: layout)
        size += g.size (c);

      auto_near b (Utils::Hook::AllocateNear (shared_ui_info, size));
      const uintptr_t p (reinterpret_cast<uintptr_t> (b.get ()));

      // See if every instruction can reach its global in the new block before
      // we move anything. AllocateNear() only makes sure that sharedUiInfo
      // reaches the block while most of the instructions count from
      // themselves. And we don't want to fail half way through, since IW4
      // would then be left reaching into two blocks, one of which we are about
      // to free. Once every instruction can reach, the move itself can't fail,
      // so there is nothing to undo.
      //
      {
        uintptr_t a (p);
        for (const auto& [g, c]: layout)
        {
          if (const reference* r = g.unreachable (a))
          {
            auto d (format ("{} at {:#x} is not within 32-bit reach of "
                            "instruction at {:#x}",
                            g.info ().name,
                            a,
                            r->place));
            throw runtime_error (std::move (d));
          }

          a += g.size (c);
        }
      }

      {
        uintptr_t a (p);
        for (const auto& [g, c]: layout)
        {
          g.relocate (a, c);
          a += g.size (c);
        }
      }

      block_ = std::move (b); // Free the old block, if any.
    }

    span<const Game::newMapArena_t> arena_globals::
    maps () const noexcept
    {
      assert (block_.get () != nullptr);

      return span<const Game::newMapArena_t> (
        static_cast<const Game::newMapArena_t*> (block_.get ()),
        Game::sharedUiInfo->mapCount);
    }

    const Game::newMapArena_t& arena_globals::
    map (const char* load_name) const noexcept
    {
      const span<const Game::newMapArena_t> ms (maps ());
      assert (!ms.empty ());

      // The names are as far apart as the elements are.
      //
      const size_t i ((load_name - ms.front ().oldMapName) /
                      sizeof (Game::newMapArena_t));

      assert (i < ms.size ());
      return ms[i];
    }

    arena_globals globals;

    // Read the arena file, making room and reading it again for as long as IW4
    // fills every slot that we have.
    //
    // Since UI_LoadArenasFromFile() drops whatever it has no room for without
    // a word, every slot being taken is our only hint that there may be more
    // arenas in the file. Of course, the file could also have exactly as many,
    // in which case we read it once more for nothing. That's harmless since
    // the function starts over each time, with the count and the memory pool
    // reset.
    //
    // We could count the arenas ourselves before IW4 reads the file, but that
    // would mean reading and parsing it twice anyway (and Maps already hooks
    // the read to swap in the arena file of a user map). Doubling the room
    // each time keeps the extra reads down to a handful even for a silly
    // number of maps.
    //
    // Note that IW4 calls us and nothing may escape, so if we can't make room,
    // then we warn and make do with the arenas that fit.
    //
    // @@ What if the arena file is bigger than the 512KB buffer that IW4 reads
    //    it into? Maps then either ignores it in favor of the one in the zone
    //    or, for a user map, cuts it short, and no amount of room on our side
    //    helps. Probably not a concern for any real arena file, so let's leave
    //    it be for now.
    //
    void
    load_arenas_from_file ()
    {
      for (;;)
      {
        Game::UI_LoadArenasFromFile ();

        const int n (globals.capacity ());

        if (*Game::ui_numArenas < n)
          break;

        try
        {
          globals.reserve (n * 2);
        }
        catch (const exception& e)
        {
          Logger::Warning (Game::CON_CHANNEL_UI,
                           "unable to load more than {} arenas: {}\n"
                           "remaining arenas in arena file are ignored\n",
                           n,
                           e.what ());
          break;
        }
      }
    }

    // UI_LoadArenas() copies the map key with I_strncpyz() limited to the 16
    // bytes of mapLoadName, which would cut a longer name short. The
    // destination is already our mapName (see map_list_field()), so all we
    // need to change is the limit.
    //
    void
    copy_map_name (char* d, const char* s, int)
    {
      Game::I_strncpyz (d, s, sizeof (Game::newMapArena_t::mapName));
    }

    // A few places in IW4 reach the map name with an 8-bit displacement from
    // the start of the element, which can't get anywhere near our mapName. Nor
    // can we make such an instruction longer without moving the code around
    // it. So what we do is leave those instructions alone and hook the call
    // that each of them passes the old name to, and then get from the old name
    // to the element and its longer name.
    //
    void
    set_map_name_dvar (const char* dvar, const char* load_name)
    {
      Game::Dvar_SetStringByName (dvar, globals.map (load_name).mapName);
    }

    int
    compare_map_name (const char* name, const char* load_name)
    {
      return Game::I_stricmp (name, globals.map (load_name).mapName);
    }

    const char*
    format_map_name (const char* format, const char* load_name)
    {
      return Game::va (format, globals.map (load_name).mapName);
    }
  }

  span<const Game::newMapArena_t> arena::
  maps ()
  {
    return globals.maps ();
  }

  const Game::newMapArena_t* arena::
  sorted_map (int i)
  {
    if (i < 0 || i >= Game::sharedUiInfo->mapCount)
      return nullptr;

    // We could look the position up in mapIndexSorted ourselves, but the list
    // may not be sorted yet and the sort is IW4's. So we let IW4 do it (we
    // don't know what the function is called), which sorts the list first if
    // necessary.
    //
    auto* f (
      Utils::Hook::Call<const Game::newMapArena_t* (int)> (0x140269970));

    return f (i);
  }

  arena::
  arena ()
  {
    // Move the globals before IW4 reads the arena file for the first time,
    // which should be well after the components are set up, so that IW4 never
    // fills the arrays in its image. If this fails, then there is no room near
    // the image and there is no point in carrying on.
    //
    globals.reserve (initial_arenas);

    auto hook ([] (uintptr_t place, auto* f)
    {
      Utils::Hook (place, f, HOOK_CALL).install ()->quick ();
    });

    hook (0x140269D52, &load_arenas_from_file);
    hook (0x140269DD6, &copy_map_name);
    hook (0x14026C01D, &set_map_name_dvar);
    hook (0x14026D1E1, &compare_map_name);
    hook (0x14026D7A4, &compare_map_name);
    hook (0x1402703A8, &format_map_name);
  }
}
