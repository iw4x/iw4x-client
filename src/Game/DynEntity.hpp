#pragma once

namespace Game
{
  typedef void(*DynEnt_UnlinkEntity_t)(DynEntityDrawType drawType, unsigned __int16 dynEntId);
  extern DynEnt_UnlinkEntity_t DynEnt_UnlinkEntity;

  typedef void(*R_UnlinkDynEnt_t)(unsigned __int16 dynEntId, DynEntityDrawType drawType);
  extern R_UnlinkDynEnt_t R_UnlinkDynEnt;

  typedef void(*UnitQuatToAxis_t)(const float* quat, float (*axis)[3]);
  extern UnitQuatToAxis_t UnitQuatToAxis;

  typedef bool(*CL_IsLocalClientActive_t)(int localClientNum);
  extern CL_IsLocalClientActive_t CL_IsLocalClientActive;

  typedef cg_s*(*CG_GetLocalClientGlobalsForEnt_t)(int entityNum);
  extern CG_GetLocalClientGlobalsForEnt_t CG_GetLocalClientGlobalsForEnt;

  typedef void(*FX_PlayOrientedEffect_t)(int localClientNum, const FxEffectDef* def, int startTime, const float* origin, const float (*axis)[3]);
  extern FX_PlayOrientedEffect_t FX_PlayOrientedEffect;

  extern const dvar_t** dynEnt_active;

  // Destroy the dynamic entity on the client, as the server tells it with
  // the j server command, and play its destroy effect.
  //
  // Note that x64 compiles out this function along with the command, so we
  // reimplement it from x86.
  //
  void DynEntCl_DestroyEvent(int localClientNum, unsigned __int16 dynEntId, DynEntityDrawType drawType, const float* hitPos, const float* hitDir);
}
