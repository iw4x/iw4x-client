#include "Weapon.hpp"

#include "GSC/Script.hpp"

extern "C"
{
  const Game::dvar_t* Weapon_BGWeaponOffHandFix;
}

namespace Components
{
  const Game::dvar_t* Weapon::CGRecoilMultiplier;
  const Game::dvar_t* Weapon::BGDisableDoubleTaps;

  std::unordered_map<std::string, Weapon::RawWeapon> Weapon::RawWeapons;
  std::uint16_t* Weapon::AltModePrimaries;

  namespace
  {
    constexpr uintptr_t ImageBase = 0x140000000;

    // The cg_s that the cgame reaches the primaries of the alternate mode
    // weapons from.
    //
    constexpr uintptr_t ClientGame = 0x140474820;

    // Instruction at place of length bytes.
    //
    struct Instruction
    {
      uintptr_t place;
      size_t length;
    };

    // Instruction that reaches the game's address target.
    //
    struct Reference
    {
      uintptr_t place;
      size_t length;
      uintptr_t target;
    };

    // Instruction whose 32-bit field holds from.
    //
    struct Value
    {
      uintptr_t place;
      size_t length;
      uint32_t from;
    };

    // A game array of count elements of size bytes at from, which we move
    // to one with room for the weapons we add.
    //
    struct Array
    {
      uintptr_t from;
      size_t size;
      size_t count;
      uintptr_t to;

      Array(uintptr_t f, size_t s, size_t c = Weapon::BASEGAME_WEAPON_LIMIT)
        : from (f), size (s), count (c),
          to (reinterpret_cast<uintptr_t>(Utils::Hook::AllocateNear(ImageBase, (c + Weapon::ADDED_WEAPONS) * s)))
      {
      }

      bool
      contains(uintptr_t a) const
      {
        return a >= from && a < from + count * size;
      }

      uintptr_t
      moved(uintptr_t a) const
      {
        assert(contains(a));

        return to + (a - from);
      }
    };

    // Point the references at the arrays that hold their targets, those that
    // reach them from the next instruction and those that reach them from
    // the image base the game keeps in a register.
    //
    void
    Retarget(std::initializer_list<const Array*> arrays,
             std::initializer_list<Reference> next,
             std::initializer_list<Reference> base)
    {
      auto moved ([&arrays] (uintptr_t target) -> uintptr_t
      {
        for (const Array* a: arrays)
        {
          if (a->contains(target))
          {
            return a->moved(target);
          }
        }

        throw std::runtime_error(std::format("no array holds {:#x}", target));
      });

      for (const Reference& r: next)
      {
        Utils::Hook::Displace(r.place, r.length, r.target, moved(r.target));
      }

      for (const Reference& r: base)
      {
        Utils::Hook::Rebase(r.place, r.length, ImageBase, r.target, moved(r.target));
      }
    }

    // Replace the 16-bit slot of an unwind code, which holds an offset or
    // size scaled down by 8.
    //
    void
    ReplaceUnwindSlot(uintptr_t place, [[maybe_unused]] uint32_t from, uint32_t to)
    {
      assert(from % 8 == 0 && to % 8 == 0 && to / 8 <= 0xFFFF);
      assert(Utils::Hook::Get<uint16_t>(place) == from / 8);

      Utils::Hook::Set<uint16_t>(place, static_cast<uint16_t>(to / 8));
    }
  }

  // An x86 server parses the weapons that have a raw file, each into its own
  // def, where a fastfile shares one def between a weapon's variants. The
  // ammo, clip and shared ammo cap indices that the server sends are
  // numbered by distinct def, so such a weapon gets its own copy here.
  //
  // Note that on x86 we load the weapon from its raw file, which x64 has no
  // parser for, and so the copy holds what the fastfile has. We also fix the
  // rumbles of a raw file there, which a fastfile has right.
  //
  // @@ Port the raw weapon file parser so that a raw file that differs from
  //    the fastfile takes effect, as it does on x86.
  //
  Game::XAssetHeader Weapon::LoadWeaponCompleteDef(const Game::XAssetType type, const char* name)
  {
    auto header (Game::DB_FindXAssetHeader(type, name));

    if (!header.weapon)
    {
      return header;
    }

    // The raw files are in the iwds, which only the search path covers.
    //
    Game::fileHandle_t file {};
    const int size (Game::FS_FOpenFileRead(std::format("weapons/mp/{}", name).c_str(), &file));

    if (file)
    {
      Game::FS_FCloseFile(file);
    }

    if (size < 0)
    {
      return header;
    }

    RawWeapon& raw (RawWeapons[name]);
    raw.complete = *header.weapon;
    raw.def = *header.weapon->weapDef;
    raw.complete.weapDef = &raw.def;

    header.weapon = &raw.complete;
    return header;
  }

  void* Weapon::CG_ClearState(void* cg, int value, std::size_t size)
  {
    std::fill_n(AltModePrimaries, WEAPON_LIMIT, static_cast<std::uint16_t>(0));
    return std::memset(cg, value, size);
  }

  void Weapon::PatchLimit()
  {
    constexpr uint32_t base (BASEGAME_WEAPON_LIMIT);
    constexpr uint32_t limit (WEAPON_LIMIT);

    // The weapon complete defs and defs, and the ammo types, shared ammo
    // caps and clips, which BG_ClearWeaponDef() clears and BG_SetupWeaponDef()
    // fills.
    //
    // Note that x64 keeps these as pointers.
    //
    {
      static const Array completeDefs (0x14043F1B0, sizeof(void*));
      static const Array weaponDefs (0x140441D70, sizeof(void*));
      static const Array ammoTypes (0x140444B30, sizeof(void*));
      static const Array sharedAmmoCaps (0x140447700, sizeof(void*));
      static const Array clips (0x14044A2C0, sizeof(void*));

      Retarget({&completeDefs, &weaponDefs, &ammoTypes, &sharedAmmoCaps, &clips},
      {
        {0x14009C256, 7, 0x14043F1B0}, {0x14009C2BD, 7, 0x14043F1B0}, {0x14009C318, 7, 0x14043F1B0},
        {0x14009C74D, 7, 0x14043F1B0}, {0x14009C872, 7, 0x14043F1B0}, {0x14009C8D3, 7, 0x14043F1B0},
        {0x14009CA12, 7, 0x14043F1B0},
        {0x14009C26A, 7, 0x140441D70}, {0x14009C2DC, 7, 0x140441D70}, {0x14009C33C, 7, 0x140441D70},
        {0x14009C7E3, 7, 0x140441D70}, {0x14009C880, 7, 0x140441D70},
        {0x14009C2A6, 7, 0x140444B30}, {0x14009C2E3, 7, 0x140444B30},
        {0x14009C292, 7, 0x140447700}, {0x14009C2EA, 7, 0x140447700}, {0x14009C803, 7, 0x140447700},
        {0x14009C27E, 7, 0x14044A2C0}, {0x14009C2F1, 7, 0x14044A2C0}
      },
      {
        {0x14009C3A0, 8, 0x14043F1B8}, {0x14009C691, 8, 0x14043F1B8}, {0x14009C6A0, 8, 0x14043F1B8},
        {0x14009CC41, 7, 0x14043F1B0}, {0x14009CE24, 8, 0x14043F1B8},
        {0x14009C3AC, 8, 0x140441D78}, {0x14009C453, 8, 0x140441D78}, {0x14009C503, 8, 0x140441D78},
        {0x14009C650, 8, 0x140441D78}, {0x14009CC36, 7, 0x140441D70}, {0x14009CDF0, 8, 0x140441D78},
        {0x14009C3D0, 8, 0x140444B30}, {0x14009C430, 8, 0x140444B30}, {0x14009C43C, 8, 0x140444B30},
        {0x14009CC88, 8, 0x140444B30}, {0x14009CCD3, 8, 0x140444B30}, {0x14009CCDD, 8, 0x140444B30},
        {0x14009C480, 8, 0x140447700}, {0x14009C4E1, 8, 0x140447700}, {0x14009C4ED, 8, 0x140447700},
        {0x14009C612, 7, 0x140447700}, {0x14009CD39, 8, 0x140447700}, {0x14009CD6C, 8, 0x140447700},
        {0x14009CD7C, 8, 0x140447700}, {0x14009CDB3, 7, 0x140447700},
        {0x14009C524, 8, 0x14044A2C0}, {0x14009C581, 8, 0x14044A2C0}, {0x14009C58E, 8, 0x14044A2C0},
        {0x14009CEC5, 8, 0x14044A2C0}, {0x14009CF0E, 8, 0x14044A2C0}, {0x14009CF18, 8, 0x14044A2C0}
      });

      // BG_ClearWeaponDef() clears them.
      //
      for (const Instruction& i: std::initializer_list<Instruction>
      {
        {0x14009C25D, 6}, {0x14009C271, 6}, {0x14009C285, 6}, {0x14009C299, 6}, {0x14009C2AD, 6}
      })
      {
        Utils::Hook::Replace32(i.place, i.length, base * sizeof(void*), limit * sizeof(void*));
      }
    }

    // Patch the ammo types and shared ammo caps on the stack, which the two
    // ammo counts of a player mark in arrays with an entry per weapon. We
    // grow their frames and move what is past the arrays along: in the first
    // one past the one array, and in the second one past the two.
    //
    // Note that on x64 the frame sizes and the offsets of the saved
    // registers are also in the unwind info, which has to match for the
    // stack to unwind through these functions.
    //
    {
      constexpr uint32_t grow (ADDED_WEAPONS * sizeof(int));

      static_assert(grow % 16 == 0);

      // BG_GetMaxPickupableAmmo()
      //
      for (const Instruction& i: std::initializer_list<Instruction>
      {
        {0x14009B4E5, 5}, {0x14009B79A, 7}
      })
      {
        Utils::Hook::Replace32(i.place, i.length, 0x1600, 0x1600 + grow);
      }

      for (const Value& v: std::initializer_list<Value>
      {
        {0x14009B506, 8, 0x1650}, {0x14009B5E7, 8, 0x1640}, {0x14009B656, 8, 0x1640},
        {0x14009B77A, 8, 0x1650}, {0x14009B792, 8, 0x1648}
      })
      {
        Utils::Hook::Replace32(v.place, v.length, v.from, v.from + grow);
      }

      Utils::Hook::Replace32(0x14009B6B2, 6, base * sizeof(int), limit * sizeof(int));

      ReplaceUnwindSlot(0x1403F1596, 0x1648, 0x1648 + grow);
      ReplaceUnwindSlot(0x1403F159A, 0x1600, 0x1600 + grow);

      // BG_GetTotalAmmoReserve()
      //
      for (const Instruction& i: std::initializer_list<Instruction>
      {
        {0x14009B867, 5}, {0x14009BA12, 7}, {0x14009BA63, 7}, {0x14009BA79, 7},
        {0x14009BAB3, 7}, {0x14009BAC4, 7}
      })
      {
        Utils::Hook::Replace32(i.place, i.length, 0x2BE0, 0x2BE0 + 2 * grow);
      }

      for (const Value& v: std::initializer_list<Value>
      {
        {0x14009B87C, 8, 0x1600}, {0x14009B990, 7, 0x1600}, {0x14009B9AB, 11, 0x1600}
      })
      {
        Utils::Hook::Replace32(v.place, v.length, v.from, v.from + grow);
      }

      for (const Value& v: std::initializer_list<Value>
      {
        {0x14009B8AE, 8, 0x2C20}, {0x14009B8C2, 8, 0x2C10}, {0x14009B8CE, 8, 0x2C28},
        {0x14009B8DD, 8, 0x2C18}, {0x14009B8FD, 8, 0x2C20}, {0x14009B9F8, 8, 0x2C28},
        {0x14009BA02, 8, 0x2C18}, {0x14009BA0A, 8, 0x2C10}
      })
      {
        Utils::Hook::Replace32(v.place, v.length, v.from, v.from + 2 * grow);
      }

      for (const Instruction& i: std::initializer_list<Instruction>
      {
        {0x14009B886, 6}, {0x14009B89A, 6}
      })
      {
        Utils::Hook::Replace32(i.place, i.length, base * sizeof(int), limit * sizeof(int));
      }

      ReplaceUnwindSlot(0x1403F1602, 0x2BE0, 0x2BE0 + 2 * grow);
      ReplaceUnwindSlot(0x1403F1616, 0x2C18, 0x2C18 + 2 * grow);
      ReplaceUnwindSlot(0x1403F161A, 0x2C28, 0x2C28 + 2 * grow);
      ReplaceUnwindSlot(0x1403F161E, 0x2C10, 0x2C10 + 2 * grow);
    }

    // The weapon names and their hashes, which two tables of the anim script
    // parser also point to.
    //
    {
      static const Array weaponStrings (0x140431270, 16);

      Retarget({&weaponStrings},
      {
        {0x140081F1C, 7, 0x140431270}, {0x1400840AD, 7, 0x140431270}, {0x1400840E6, 7, 0x140431270}
      },
      {});

      for (const uintptr_t place: {0x14041CA58, 0x14041CB48})
      {
        assert(Utils::Hook::Get<uintptr_t>(place) == weaponStrings.from);

        Utils::Hook::Set<uintptr_t>(place, weaponStrings.to);
      }

      for (const Instruction& i: std::initializer_list<Instruction>
      {
        {0x140081F0F, 6}, {0x1400840ED, 6}
      })
      {
        Utils::Hook::Replace32(i.place, i.length, base * 16, limit * 16);
      }
    }

    // The cgame's weapons, static and of the local client.
    //
    // Note that a loop over the array before the latter ends at a field of
    // its last element, which only happens to lie within ours.
    //
    {
      static const Array cg_weaponsStaticArray (0x1406B29A0, 24);
      static const Array cg_weaponsArray (0x140589720, 56);

      Retarget({&cg_weaponsStaticArray, &cg_weaponsArray},
      {
        {0x1400C836E, 7, 0x1406B29A0}, {0x1400C83D7, 7, 0x1406B29A0}, {0x1400C847E, 7, 0x1406B29A0},
        {0x1400D652C, 7, 0x1406B29A0}, {0x1400DA1C8, 7, 0x1406B29A0},
        {0x1400C405B, 7, 0x140589730}, {0x1400C49EC, 7, 0x140589720}, {0x1400C74F4, 7, 0x140589720},
        {0x1400C7A21, 7, 0x140589720}, {0x1400C7AAB, 7, 0x140589720}, {0x1400C7C9F, 7, 0x140589720},
        {0x1400C818E, 7, 0x140589720}, {0x1400D6502, 7, 0x140589720}, {0x1400DA1DA, 7, 0x140589720},
        {0x1400DCEDE, 7, 0x140589720}, {0x1400DD062, 7, 0x140589720}
      },
      {
        {0x1400C4A0C, 7, 0x1406B29A0}, {0x1400D4B3D, 8, 0x1406B29A0},
        {0x1400C056B, 8, 0x140589730}, {0x1400C75F6, 8, 0x140589739}, {0x1400DB1A3, 8, 0x140589740},
        {0x1400DB28F, 8, 0x140589740}
      });

      for (const Instruction& i: std::initializer_list<Instruction>
      {
        {0x1400D6533, 6}, {0x1400DA1CF, 6}
      })
      {
        Utils::Hook::Replace32(i.place, i.length, base * 24, limit * 24);
      }

      for (const Instruction& i: std::initializer_list<Instruction>
      {
        {0x1400D650B, 7}, {0x1400D6512, 6}, {0x1400DA1E3, 7}, {0x1400DA1EA, 6}
      })
      {
        Utils::Hook::Replace32(i.place, i.length, base * 56, limit * 56);
      }
    }

    // The hint materials in cgMedia_t, the weapons' from the fifth on, which
    // CG_RegisterWeapon(), CG_RegisterGraphics() and CG_DrawCursorhint()
    // reach. The latter takes a hint up to one past the last weapon.
    //
    // Note that the effects that the cgame looks up by the three digits of a
    // configstring seem to reach into it but do not: the lookups subtract
    // the digits' character codes from where they point.
    //
    {
      static const Array hintMaterials (0x140469260, sizeof(void*), base + 5);

      Retarget({&hintMaterials},
      {
        {0x1400D9937, 7, 0x140469278}, {0x1400D994F, 7, 0x140469280}
      },
      {
        {0x1400C4BFA, 8, 0x140469280}, {0x1400DB10A, 8, 0x140469260}
      });

      Utils::Hook::Replace32(0x1400DB130, 5, base - 1, limit - 1);
    }

    // The primary weapon of each alternate mode weapon, which cg_s holds and
    // the cgame reaches relative to it. We move it out and clear it along
    // with cg_s, and the copy that the cgame keeps across a renderer restart
    // along with it.
    //
    {
      static const Array primaries (ClientGame + 0x7592C, sizeof(uint16_t));
      static const Array saved (0x1406CC068, sizeof(uint16_t));

      AltModePrimaries = reinterpret_cast<uint16_t*>(primaries.to);

      for (const Instruction& i: std::initializer_list<Instruction>
      {
        {0x1400C0FEC, 8}, {0x1400C3D44, 8}, {0x1400C41C6, 9}, {0x1400C41FC, 9},
        {0x1400C424F, 9}, {0x1400C52AA, 9}, {0x1400C52B5, 10}, {0x1400C52C9, 9},
        {0x1400C536F, 9}, {0x1400C53F2, 9}, {0x1400C8744, 9}, {0x1400C877A, 8}
      })
      {
        Utils::Hook::Rebase(i.place, i.length, ClientGame, primaries.from, primaries.to);
      }

      Retarget({&primaries, &saved},
      {
        {0x1400D6E1A, 7, 0x1404EA14C}, {0x1400D9FF7, 7, 0x1404EA14C}, {0x1400F6B66, 7, 0x1404EA14C},
        {0x1400D6E27, 7, 0x1406CC068}, {0x1400D9FEA, 7, 0x1406CC068}
      },
      {});

      for (const Instruction& i: std::initializer_list<Instruction>
      {
        {0x1400D6E34, 6}, {0x1400DA004, 6}
      })
      {
        Utils::Hook::Replace32(i.place, i.length, base * sizeof(uint16_t), limit * sizeof(uint16_t));
      }

      for (const uintptr_t place: {0x1400D64BA, 0x1400DA2BC})
      {
        Utils::Hook(place, CG_ClearState, HOOK_CALL).install()->quick();
      }
    }

    // The game splits an item index into its weapon and model, as the
    // remainder and quotient of the limit, dividing by multiplying with the
    // reciprocal and shifting by 41 in all. We do the same with ours.
    //
    // Note that on x86 the limit is a power of two times the game's and so
    // we shift further instead.
    //
    {
      constexpr uint32_t reciprocal (static_cast<uint32_t>(((1ull << 41) + base - 1) / base));
      constexpr uint32_t ours (static_cast<uint32_t>(((1ull << 41) + limit - 1) / limit));

      static_assert(reciprocal == 0x5D9F7391);

      for (const Instruction& i: std::initializer_list<Instruction>
      {
        {0x14008918A, 5}, {0x14008AB13, 5}, {0x1400D54E8, 5}, {0x1400D550D, 5},
        {0x1400E3383, 5}, {0x1400E33B0, 5}, {0x1400E345B, 5}, {0x14016C329, 5},
        {0x14016D552, 5}, {0x14016D793, 5}, {0x14016DE59, 5}, {0x14016DF09, 5},
        {0x14016DFF3, 5}, {0x14016ECA9, 5}, {0x14016EDC4, 5}, {0x14016EF13, 5},
        {0x14016EF4B, 5}, {0x14016F073, 5}, {0x14016F0B3, 5}, {0x14017DA47, 5},
        {0x14018C12A, 5}, {0x14018C2E9, 5}, {0x14018C33E, 5}, {0x140194FBE, 5},
        {0x140195034, 5}
      })
      {
        Utils::Hook::Replace32(i.place, i.length, reciprocal, ours);
      }
    }

    // The limit, where the game multiplies by it to make an item index, on
    // the weapons that the cgame registers from the configstrings and clears,
    // and on the index past which a kill icon is a means of death's and the
    // offset of the weapon of an alternate model.
    //
    for (const Instruction& i: std::initializer_list<Instruction>
    {
      {0x14008919B, 6}, {0x14008A3D0, 6}, {0x14008AB24, 6}, {0x14008D673, 6},
      {0x1400D54FB, 6}, {0x1400E3394, 6}, {0x1400E33C2, 6}, {0x1400E3478, 6},
      {0x14016C34E, 6}, {0x14016C58B, 7}, {0x14016D568, 6}, {0x14016D7A4, 6},
      {0x14016DE6A, 6}, {0x14016DF22, 6}, {0x14016E004, 6}, {0x14016E823, 7},
      {0x14016EDDC, 7}, {0x14016EF24, 6}, {0x14016F084, 6}, {0x14017DA5F, 6},
      {0x14018C13B, 6}, {0x14018C300, 6}, {0x14018C34F, 6}, {0x140194FCF, 6},
      {0x14019504B, 6},
      {0x1400C499A, 5}, {0x1400D693E, 6}, {0x1400AEFBC, 6}, {0x1401A39CC, 5}
    })
    {
      Utils::Hook::Replace32(i.place, i.length, base, limit);
    }

    Utils::Hook::Replace32(0x1400AEFD5, 6, static_cast<uint32_t>(-static_cast<int32_t>(base + 8)), static_cast<uint32_t>(-static_cast<int32_t>(limit + 8)));

    // CG_EntityEvent() takes the weapon of a weapon pickup or change only if
    // its index less one is within the limit's.
    //
    Utils::Hook::Replace32(0x1400AD429, 5, base - 2, limit - 2);
  }

  void* Weapon::LoadNoneWeaponHook()
  {
    // load anim scripts now, rather than a bit later on
    Utils::Hook::Call<void()>(0x14009C0A0)();

    return Game::DB_FindXAssetHeader(Game::XAssetType::ASSET_TYPE_WEAPON, "none").data;
  }

  void Weapon::BG_WeaponFireRecoil_Stub(
    void* ps,
    float* recoilSpeed,
    float* kickAVel,
    unsigned int* holdrand,
    Game::PlayerHandIndex hand
  )
  {
    float adjustedRecoilSpeed[3]{};
    float adjustedKick[3]{};

    Utils::Hook::Call<void(void*, float*, float*, unsigned int*, Game::PlayerHandIndex)>(0x14009E300)(
      ps,
      adjustedRecoilSpeed,
      adjustedKick,
      holdrand,
      hand
    );

    for (size_t axis = 0; axis < 3; axis++)
    {
      // Note that the game adds the gun kick to the speed left over from the
      // previous shots (so that it builds up during automatic fire) while it
      // assigns the view kick. So we do the same.
      //
      recoilSpeed [axis] += adjustedRecoilSpeed[axis] * CGRecoilMultiplier->current.value;
      kickAVel [axis] = adjustedKick[axis] * CGRecoilMultiplier->current.value;
    }
  }

  // Drop the weapon for dropItem() unless it has no world model to show.
  //
  // Some weapons are only ever held, so they have no world model (say, the
  // airdrop marker) and dropping one is a fatal error when spawning its item.
  // The game's own drops only consider primary weapons, so it never runs into
  // this, but scripts can ask for any weapon. In particular, the stock scripts
  // drop the weapon a player was holding when spawning without checking that it
  // can be dropped. We've seen this happen with Bot Warfare bots and that's our
  // best guess: a bot somehow spawns with its killstreak marker in hand and
  // takes the whole server down when it dies.
  //
  // So we refuse such a drop the same way Drop_Weapon() refuses an alternate
  // fire weapon: warn and drop nothing (and so dropItem() returns undefined),
  // leaving the player with the weapon.
  //
  // @@ The real fix belongs in Bot Warfare (its bots shouldn't end up holding
  //    the marker like this). Remove this once it's fixed there?
  //
  Game::gentity_s* Weapon::Drop_Weapon_Hk(Game::gentity_s* ent, unsigned int weapon, unsigned int model)
  {
    const auto* def (Game::BG_GetWeaponDef(weapon));
    if (!def->worldModel[static_cast<std::uint8_t>(model)])
    {
      Logger::Warning(Game::CON_CHANNEL_SCRIPT, "Drop_Weapon(): Trying to drop weapon '{}' with no world model.\n", Game::BG_GetWeaponName(weapon));
      return nullptr;
    }

    return Utils::Hook::Call<Game::gentity_s*(Game::gentity_s*, unsigned int, unsigned int)>(0x14016C540)(ent, weapon, model);
  }

  void Weapon::PlayerCmd_InitialWeaponRaise(const Game::scr_entref_t entref)
  {
    auto* ent = GSC::Script::Scr_GetPlayerEntity(entref);
    const auto* weapon = Game::Scr_GetString(0);
    const auto index = Game::G_GetWeaponIndexForName(weapon);

    auto* ps = &ent->client->ps;
    if (!Game::BG_IsWeaponValid(ps, index))
    {
      Game::Scr_Error(Utils::String::VA("invalid InitialWeaponRaise: %s", weapon));
      return;
    }

    assert(ps);

    if (!index)
    {
      return;
    }

    auto* equippedWeapon = Game::BG_GetEquippedWeaponState(ps, index);
    if (!equippedWeapon)
    {
      return;
    }

    equippedWeapon->usedBefore = false;

    // Note that on x86 we call the body of PlayerCmd_SwitchToWeapon(), which
    // x64 inlines into it. It reads the same weapon name argument.
    //
    Game::PlayerCmd_SwitchToWeapon(entref);
  }

  void Weapon::PlayerCmd_FreezeControlsAllowLook(const Game::scr_entref_t entref)
  {
    const auto* ent = GSC::Script::Scr_GetPlayerEntity(entref);

    if (Game::Scr_GetInt(0))
    {
      ent->client->ps.weapCommon.weapFlags |= Game::PWF_DISABLE_WEAPONS;
      ent->client->flags |= Game::CF_BIT_DISABLE_USABILITY;
    }
    else
    {
      ent->client->ps.weapCommon.weapFlags &= ~Game::PWF_DISABLE_WEAPONS;
      ent->client->flags &= ~Game::CF_BIT_DISABLE_USABILITY;
    }
  }

  void Weapon::AddScriptMethods()
  {
    GSC::Script::AddMethod("DisableWeaponPickup", [](const Game::scr_entref_t entref)
      {
        const auto* ent = GSC::Script::Scr_GetPlayerEntity(entref);

        ent->client->ps.weapCommon.weapFlags |= Game::PWF_DISABLE_WEAPON_PICKUP;
      });

    GSC::Script::AddMethod("EnableWeaponPickup", [](const Game::scr_entref_t entref)
      {
        const auto* ent = GSC::Script::Scr_GetPlayerEntity(entref);

        ent->client->ps.weapCommon.weapFlags &= ~Game::PWF_DISABLE_WEAPON_PICKUP;
      });

    // PlayerCmd_AreControlsFrozen GSC function from Black Ops 2
    GSC::Script::AddMethod("AreControlsFrozen", [](Game::scr_entref_t entref) // Usage: self AreControlsFrozen();
      {
        const auto* ent = GSC::Script::Scr_GetPlayerEntity(entref);
        Game::Scr_AddBool((ent->client->flags & Game::CF_BIT_FROZEN) != 0);
      });

    GSC::Script::AddMethod("InitialWeaponRaise", PlayerCmd_InitialWeaponRaise);
    GSC::Script::AddMethod("FreezeControlsAllowLook", PlayerCmd_FreezeControlsAllowLook);
  }

  void Weapon::PM_Weapon_stub(Game::pmove_s* pm, Game::pml_t* pml)
  {
    if (BGDisableDoubleTaps && BGDisableDoubleTaps->current.enabled)
    {
      if (pm && pm->ps)
      {
        if (pm->cmd.weapon == pm->ps->weapCommon.weapon &&
          (pm->ps->weapState[Game::WEAPON_HAND_RIGHT].weaponState == Game::WEAPON_DROPPING
            || pm->ps->weapState[Game::WEAPON_HAND_RIGHT].weaponState == Game::WEAPON_DROPPING_QUICK
            || pm->ps->weapState[Game::WEAPON_HAND_RIGHT].weaponState == Game::WEAPON_DROPPING_ALT))
        {
          // change each hand's weapon data
          for (int index = 0; index < Game::NUM_WEAPON_HANDS; index++)
          {
            pm->ps->weapState[index].weaponState = Game::WEAPON_RAISING;
            pm->ps->weapState[index].weaponTime = Game::BG_GetWeaponDef(pm->ps->weapCommon.weapon)->quickRaiseTime;
            pm->ps->weapState[index].weapAnim = Game::WEAP_ANIM_QUICK_DROP;
          }
        }
      }
    }

    Game::PM_Weapon(pm, pml);
  }

  Weapon::Weapon()
  {
    // Weapon.asm has its own copy of this offset.
    //
    AssertOffset(Game::dvar_t, current, 0x10);

    if (!Flags::HasFlag("steamdemo") && !Flags::HasFlag("retaildemo"))
    {
      // Steam version uses a limit of 1400 weapons, the same as x64, and
      // other clients use the default limit of 1200 weapons
      PatchLimit();
    }

    // Note that on x86 we load weapons from their raw files, which also
    // takes the warning about a missing raw file, the existence check that
    // follows and the double load with fs_game out of the way. x64 loads
    // them from the fastfiles only, so we make our copies where it does.
    //
    Utils::Hook(0x14009C950, LoadWeaponCompleteDef, HOOK_CALL).install()->quick(); // BG_GetWeaponIndexForName

    // Automatically register weapons, even if the level is not loading
    Utils::Hook::Nop(0x1401884B7, 2); // G_GetWeaponIndexForName
    Utils::Hook::Set<BYTE>(0x1401AC05B, 0xEB); // G_ModelIndex

    // Weapon swap fix
    Utils::Hook::Nop(0x14009C302, 5);
    Utils::Hook(0x14009C080, LoadNoneWeaponHook, HOOK_JUMP).install()->quick();

    // MW3-style weapon swapping mechanics
    BGDisableDoubleTaps = Game::Dvar_RegisterBool("bg_disableDoubleTaps", false, Game::DVAR_CODINFO, "Enables MW3-style weapon swapping mechanics");
    Utils::Hook(0x140094B86, PM_Weapon_stub, HOOK_CALL).install()->quick(); // PmoveSingle
    Utils::Hook(0x140094DAA, PM_Weapon_stub, HOOK_CALL).install()->quick(); // ^
    Utils::Hook(0x140095172, PM_Weapon_stub, HOOK_CALL).install()->quick(); // ^

    // Note that on x86 we also clear the weapons independently from fs_game,
    // freeing the ones loaded from raw files. x64 has no such weapons to
    // free and clears the rest regardless.

    // The game reads the def of the weapon to select without checking that
    // the server registered it. So we select nothing in that case, the same
    // as on x86.
    //
    // Note that x64 inlines CG_UpdatePrimaryForAltModeWeapon() into
    // CG_SelectWeaponIndex(), whose check of the def comes after it on x86
    // and so we check it once, where the former reads it.
    //
    Utils::Hook(0x1400C5272, Weapon_CG_SelectWeaponIndex_Stub, HOOK_CALL).install()->quick();

    // The game draws a weapon item from the weapon and model indexes packed
    // in the entity's item index, but doesn't check that the weapon is
    // actually loaded and crashes if it isn't. So we skip such an item, the
    // same as the game does when the weapon has no model for that index.
    //
    // @@ Not clear how we end up with an item for a weapon we don't have.
    //    Perhaps we should fail with a fatal error instead (which the crash
    //    reporter would offer to send us) so that we can find out?
    //
    Utils::Hook(0x1400D5505, Weapon_CG_DrawItem_Stub, HOOK_CALL).install()->quick();

    AssertOffset(Game::playerState_s, Game::playerState_s::weapCommon.weapFlags, 0x2BC);
    Utils::Hook(0x14008D76E, Weapon_WeaponEntCanBeGrabbed_Stub, HOOK_JUMP).install()->quick();

    // Javelin fix (PM_Weapon_OffHandEnd)
    //
    // Note that x64 already resets grenadeTimeLeft after it, which we leave
    // to the dvar as on x86.
    //
    AssertOffset(Game::playerState_s, grenadeTimeLeft, 0x34);
    Weapon_BGWeaponOffHandFix = Game::Dvar_RegisterBool("bg_weaponOffHandFix", true, Game::DVAR_CODINFO, "Reset grenadeTimeLeft after using off hand weapon");
    Utils::Hook(0x140098EA0, Weapon_JavelinResetHook_Stub, HOOK_CALL).install()->quick();

    // The game creates the view model tree from the animations of the hand
    // when the weapon is dual wielded. But it doesn't create those for a
    // weapon with noDualWield set and so, if a server or script hands out such
    // a weapon dual wielded anyway, the tree ends up with no animations and
    // the game crashes as soon as it animates it. So we fall back to the
    // weapon's own animations in this case, which the game makes sure exist.
    //
    // Note that rax points to cg_weaponsStatic and rsi is three times the
    // weapon, the index of its animations followed by those of each hand.
    //
    Utils::Hook(0x1400C83E2, Weapon_CG_CreateViewModelTree_Stub, HOOK_CALL).install()->quick();

    CGRecoilMultiplier = Game::Dvar_RegisterFloat("cg_recoilMultiplier",
      1.0f, 0.0f, 1000.0f, Game::DVAR_CHEAT,
      "The scale applied to the player recoil when firing");
    Utils::Hook(0x1400C32E1, BG_WeaponFireRecoil_Stub, HOOK_CALL).install()->quick();
    Utils::Hook(0x14019420C, BG_WeaponFireRecoil_Stub, HOOK_CALL).install()->quick();

    // Don't drop a weapon with no world model (see Drop_Weapon_Hk() for details).
    AssertOffset(Game::WeaponDef, worldModel, 0x2E0);
    Utils::Hook(0x1401974FC, Drop_Weapon_Hk, HOOK_CALL).install()->quick(); // PlayerCmd_dropItem

    AddScriptMethods();
  }
}
