#include "Bullet.hpp"

namespace Components
{
  Dvar::Var Bullet::BGSurfacePenetration;
  const Game::dvar_t* Bullet::BGBulletRange;

  float Bullet::BG_GetSurfacePenetrationDepthStub(const Game::WeaponDef* weapDef, int surfaceType)
  {
    assert(weapDef);
    assert(weapDef->penetrateType != Game::PENETRATE_TYPE_NONE);
    AssertIn(weapDef->penetrateType, Game::PENETRATE_TYPE_COUNT);
    AssertIn(surfaceType, Game::SURF_TYPE_COUNT);

    const auto penetrationDepth = BGSurfacePenetration.get<float>();
    if (penetrationDepth > 0.0f)
    {
      // Custom depth
      return penetrationDepth;
    }

    // Game's code
    if (surfaceType != Game::SURF_TYPE_DEFAULT)
    {
      return (*Game::penetrationDepthTable)[weapDef->penetrateType][surfaceType];
    }

    return 0.0f;
  }

  void Bullet::BG_srand_Hk(unsigned int* pHoldrand)
  {
    *pHoldrand = static_cast<unsigned int>(std::rand());
  }

  Bullet::Bullet()
  {
    BGSurfacePenetration = Dvar::Register<float>("bg_surfacePenetration", 0.0f,
      0.0f, std::numeric_limits<float>::max(), Game::DVAR_CODINFO, "Set to a value greater than 0 to override the surface penetration depth");
    BGBulletRange = Game::Dvar_RegisterFloat("bg_bulletRange", 8192.0f,
      0.0f, std::numeric_limits<float>::max(), Game::DVAR_CODINFO, "Max range used when calculating the bullet end position");

    Utils::Hook(0x14009C820, BG_GetSurfacePenetrationDepthStub, HOOK_JUMP).install()->quick();

    // Load range of bullets without a penetrating weapon from bg_bulletRange
    // instead of the constant in Bullet_Fire.
    //
    // Note that the game allocates dvars in its own image, so the operand
    // reaches the value directly.
    //
    Utils::Hook::Displace(0x140161BC5, 8, 0x14036EA4C,
                          reinterpret_cast<uintptr_t>(&BGBulletRange->current.value));

    Utils::Hook(0x140161BF2, BG_srand_Hk, HOOK_CALL).install()->quick();
  }
}
