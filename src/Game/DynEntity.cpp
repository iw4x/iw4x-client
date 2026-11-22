#include "DynEntity.hpp"

namespace Game
{
  DynEnt_UnlinkEntity_t DynEnt_UnlinkEntity = DynEnt_UnlinkEntity_t(0x140136780);
  R_UnlinkDynEnt_t R_UnlinkDynEnt = R_UnlinkDynEnt_t(0x140025540);
  UnitQuatToAxis_t UnitQuatToAxis = UnitQuatToAxis_t(0x14027B7C0);
  CL_IsLocalClientActive_t CL_IsLocalClientActive = CL_IsLocalClientActive_t(0x1400F3130);
  CG_GetLocalClientGlobalsForEnt_t CG_GetLocalClientGlobalsForEnt = CG_GetLocalClientGlobalsForEnt_t(0x1400C3C10);
  FX_PlayOrientedEffect_t FX_PlayOrientedEffect = FX_PlayOrientedEffect_t(0x140158630);

  const dvar_t** dynEnt_active = reinterpret_cast<const dvar_t**>(0x1415FB2E8);

  namespace
  {
    // The client flag that DynEntCl_DestroyEvent() clears. Note that the
    // damage that destroys a dynamic entity on the client clears it along
    // with that of its physics object.
    //
    constexpr unsigned __int16 DYNENT_CL_VISIBLE = 1 << 1;
  }

  void DynEntCl_DestroyEvent(const int localClientNum, const unsigned __int16 dynEntId, const DynEntityDrawType drawType, [[maybe_unused]] const float* hitPos, [[maybe_unused]] const float* hitDir)
  {
    assert(drawType == DYNENT_DRAW_MODEL || drawType == DYNENT_DRAW_BRUSH);

    if (!(*dynEnt_active)->current.enabled)
    {
      return;
    }

    // Note that x86 checks the flags and the client number of the predicted
    // player state again, which CG_GetLocalClientGlobalsForEnt() has already
    // checked. So we don't bother.
    //
    if (CG_GetLocalClientGlobalsForEnt(static_cast<int>(ENTITYNUM_NONE)) != nullptr)
    {
      const auto& ps(cgArray[0].predictedPlayerState);

      if (!(ps.otherFlags & (POF_FOLLOW | POF_PLAYER)) || ps.clientNum != static_cast<int>(ENTITYNUM_NONE))
      {
        return;
      }
    }
    else if (localClientNum != CL_ControllerIndexFromClientNum(0))
    {
      return;
    }

    auto& client(cm->dynEntClientList[drawType][dynEntId]);
    const auto& def(cm->dynEntDefList[drawType][dynEntId]);

    client.flags &= ~DYNENT_CL_VISIBLE;

    DynEnt_UnlinkEntity(drawType, dynEntId);
    R_UnlinkDynEnt(dynEntId, drawType);

    const auto& pose(cm->dynEntPoseList[drawType][dynEntId]);

    if (def.destroyFx == nullptr)
    {
      return;
    }

    float axis[3][3];
    UnitQuatToAxis(pose.pose.quat, axis);

    if (CL_IsLocalClientActive(0))
    {
      FX_PlayOrientedEffect(0, def.destroyFx, cgArray[0].time, pose.pose.origin, axis);
    }
  }
}
