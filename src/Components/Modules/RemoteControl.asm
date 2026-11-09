.686
.model flat, C
option casemap:none

EXTERN RemoteControl_ScaleMouseMovement:PROC

.code

RemoteControl_CL_GetMouseMovementWithSensitivity_Stub PROC
  push    dword ptr [esp+8h]
  push    dword ptr [esp+8h]
  call    dword ptr [gameCL_GetMouseMovementWithSensitivity]
  add     esp, 8h

  pushad

  push    dword ptr [esp+20h+8h]
  push    dword ptr [esp+24h+4h]
  call    RemoteControl_ScaleMouseMovement
  add     esp, 8h

  popad

  ret

gameCL_GetMouseMovementWithSensitivity:
  dd      5A6170h
RemoteControl_CL_GetMouseMovementWithSensitivity_Stub ENDP

END
