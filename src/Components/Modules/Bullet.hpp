#pragma once

namespace Components
{
  class Bullet : public Component
  {
  public:
    Bullet();

  private:
    static Dvar::Var BGSurfacePenetration;
    static const Game::dvar_t* BGBulletRange;

    static float BG_GetSurfacePenetrationDepthStub(const Game::WeaponDef* weapDef, int surfaceType);

    static void BG_srand_Hk(unsigned int* pHoldrand);
  };
}
