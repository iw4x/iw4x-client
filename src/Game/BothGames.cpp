#include "BothGames.hpp"

namespace Game
{
  BG_GetNumWeapons_t BG_GetNumWeapons = BG_GetNumWeapons_t(0x14009C7D0);
  BG_GetWeaponName_t BG_GetWeaponName = BG_GetWeaponName_t(0x14009CA10);
  BG_LoadWeaponDef_LoadObj_t BG_LoadWeaponDef_LoadObj = BG_LoadWeaponDef_LoadObj_t(0x140188770);
  BG_GetWeaponDef_t BG_GetWeaponDef = BG_GetWeaponDef_t(0x14009C880);
  BG_GetEntityTypeName_t BG_GetEntityTypeName = BG_GetEntityTypeName_t(0x14008A3E0);
  BG_IsWeaponValid_t BG_IsWeaponValid = BG_IsWeaponValid_t(0x14009D880);
  BG_GetEquippedWeaponIndex_t BG_GetEquippedWeaponIndex = BG_GetEquippedWeaponIndex_t(0x1400C0010);
  BG_GetViewModelWeaponIndex_t BG_GetViewModelWeaponIndex = BG_GetViewModelWeaponIndex_t(0x14009C850);
  BG_GetEquippedWeaponState_t BG_GetEquippedWeaponState = BG_GetEquippedWeaponState_t(0x140187680);
  BG_PlayerHasWeapon_t BG_PlayerHasWeapon = BG_PlayerHasWeapon_t(0x14008AB80);
  BG_GetWeaponCompleteDef_t BG_GetWeaponCompleteDef = BG_GetWeaponCompleteDef_t(0x14009C870);
  BG_TakePlayerWeapon_t BG_TakePlayerWeapon = BG_TakePlayerWeapon_t(0x1401887D0);
}
