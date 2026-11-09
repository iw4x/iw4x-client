.686
.model flat, C
option casemap:none

EXTERN PlayerMovement_PlayerDuckedSpeedScale:DWORD
EXTERN PlayerMovement_PlayerProneSpeedScale:DWORD
EXTERN PlayerMovement_CGNoclipScaler:DWORD
EXTERN PlayerMovement_CGUfoScaler:DWORD
EXTERN PlayerMovement_PlayerSpectateSpeedScale:DWORD
EXTERN PlayerMovement_BGBounces:DWORD
EXTERN PlayerMovement_BGBouncesAllAngles:DWORD
EXTERN PlayerMovement_BGBunnyHopAuto:DWORD
EXTERN PlayerMovement_BGLadderFixedInput:DWORD
EXTERN PlayerMovement_BGSprintIgnoreRepress:DWORD
EXTERN PlayerMovement_BGOmnimovement:DWORD
EXTERN PlayerMovement_BGDive:DWORD
EXTERN PlayerMovement_BGOmnimovementDive:DWORD

EXTERN PlayerMovement_ComputeHorizontalIntent:PROC
EXTERN PlayerMovement_ApplyStockSprintStrafeScale:PROC

CMD_BUTTON_UP equ 400h

.code

PlayerMovement_PM_PlayerDuckedSpeedScaleStub PROC
  push    eax
  mov     eax, PlayerMovement_PlayerDuckedSpeedScale
  fld     dword ptr [eax+10h]
  pop     eax

  pop     ecx
  ret
PlayerMovement_PM_PlayerDuckedSpeedScaleStub ENDP

PlayerMovement_PM_PlayerProneSpeedScaleStub PROC
  push    eax
  mov     eax, PlayerMovement_PlayerProneSpeedScale
  fld     dword ptr [eax+10h]
  pop     eax

  pop     ecx
  ret
PlayerMovement_PM_PlayerProneSpeedScaleStub ENDP

PlayerMovement_PM_MoveScale_Noclip PROC
  mov     eax, PlayerMovement_CGNoclipScaler
  fld     dword ptr [eax+10h]
  fmul    dword ptr [esp+0Ch]
  fstp    dword ptr [esp+0Ch]

  push    56F43Ah
  ret
PlayerMovement_PM_MoveScale_Noclip ENDP

PlayerMovement_PM_MoveScale_Ufo PROC
  mov     eax, PlayerMovement_CGUfoScaler
  fld     dword ptr [eax+10h]
  fmul    dword ptr [esp+0Ch]
  fstp    dword ptr [esp+0Ch]

  push    56F44Dh
  ret
PlayerMovement_PM_MoveScale_Ufo ENDP

PlayerMovement_PM_MoveScale_Spectate PROC
  mov     eax, PlayerMovement_PlayerSpectateSpeedScale
  fld     dword ptr [eax+10h]
  fmul    dword ptr [esp+0Ch]
  fstp    dword ptr [esp+0Ch]

  push    56F462h
  ret
PlayerMovement_PM_MoveScale_Spectate ENDP

PlayerMovement_PM_StepSlideMoveStub PROC
  push    eax

  mov     eax, PlayerMovement_BGBounces
  mov     eax, dword ptr [eax+10h]
  test    eax, eax

  pop     eax

  jle     noBounce

  push    eax

  mov     eax, PlayerMovement_BGBouncesAllAngles
  mov     eax, dword ptr [eax+10h]
  cmp     eax, 2

  pop     eax

  jne     regularBounce

  push    4B1B7Dh
  ret

regularBounce:
  push    4B1B34h
  ret

noBounce:
  cmp     dword ptr [esp+24h], 0
  push    4B1B32h
  ret
PlayerMovement_PM_StepSlideMoveStub ENDP

PlayerMovement_PM_ProjectVelocityStub PROC
  push    eax
  mov     eax, PlayerMovement_BGBouncesAllAngles
  mov     eax, dword ptr [eax+10h]
  test    eax, eax
  pop     eax

  je      noBounce

  push    417B6Fh
  ret

noBounce:
  fstp    st(0)
  pop     esi
  add     esp, 10h
  ret
PlayerMovement_PM_ProjectVelocityStub ENDP

PlayerMovement_Jump_Check_Stub PROC
  push    eax
  mov     eax, PlayerMovement_BGBunnyHopAuto
  cmp     byte ptr [eax+10h], 1
  pop     eax

  je      autoHop

  test    dword ptr [ebp+30h], CMD_BUTTON_UP
  push    4E9890h
  ret

autoHop:
  push    4E989Fh
  ret
PlayerMovement_Jump_Check_Stub ENDP

PlayerMovement_PM_LadderMove_PitchStub PROC
  mov     eax, PlayerMovement_BGLadderFixedInput
  test    eax, eax
  jz      stockPath
  cmp     byte ptr [eax+10h], 1
  jne     stockPath

  fld1
  fstp    dword ptr [esp+0Ch]
  push    574034h
  ret

stockPath:
  fld     dword ptr [ebp+8h]
  fadd    qword ptr ds:[70D1E8h]
  push    573FF8h
  ret
PlayerMovement_PM_LadderMove_PitchStub ENDP

PlayerMovement_PM_UpdateSprint_RepressCallStub PROC
  push    eax
  pushfd

  mov     eax, PlayerMovement_BGSprintIgnoreRepress
  test    eax, eax
  jz      stockPath
  cmp     byte ptr [eax+10h], 1
  jne     stockPath

  popfd
  pop     eax
  push    56EF64h
  ret

stockPath:
  popfd
  pop     eax
  mov     ecx, ebp
  mov     eax, edi
  push    56EF5Eh
  push    56ECD0h
  ret
PlayerMovement_PM_UpdateSprint_RepressCallStub ENDP

PlayerMovement_CL_KeyMove_SprintBit_Stub PROC
  push    eax
  mov     eax, PlayerMovement_BGOmnimovement
  test    eax, eax
  jz      doStock
  cmp     byte ptr [eax+10h], 0
  jz      doStock
  pop     eax

  push    5A6061h
  ret

doStock:
  pop     eax

  cmp     byte ptr [esi+4Ch], 0
  jnz     stockSkip

  push    5A6061h
  ret

stockSkip:
  push    5A6084h
  ret
PlayerMovement_CL_KeyMove_SprintBit_Stub ENDP

PlayerMovement_PM_SprintStartInterferingButtons_Stub PROC
  pushfd
  push    eax
  push    ecx
  push    edx

  movsx   eax, byte ptr [ebp+1Fh]
  push    eax
  push    esi
  call    PlayerMovement_ComputeHorizontalIntent
  add     esp, 8

  mov     esi, eax

  pop     edx
  pop     ecx
  pop     eax
  popfd

  push    56ED10h
  ret
PlayerMovement_PM_SprintStartInterferingButtons_Stub ENDP

PlayerMovement_PM_SprintEndingButtons_Stub PROC
  pushfd
  push    eax
  push    ecx

  movsx   eax, byte ptr [ebp+1Fh]
  push    eax
  push    edx
  call    PlayerMovement_ComputeHorizontalIntent
  add     esp, 8

  mov     edx, eax

  pop     ecx
  pop     eax
  popfd

  push    56ED80h
  ret
PlayerMovement_PM_SprintEndingButtons_Stub ENDP

PlayerMovement_PM_WalkMove_SprintStrafeStub PROC
  push    eax
  mov     eax, PlayerMovement_BGOmnimovement
  test    eax, eax
  jz      doStock
  cmp     byte ptr [eax+10h], 0
  jz      doStock
  pop     eax

  push    573289h
  ret

doStock:
  pop     eax

  push    eax
  push    ecx
  push    edx
  push    edi
  call    PlayerMovement_ApplyStockSprintStrafeScale
  add     esp, 4
  pop     edx
  pop     ecx
  pop     eax

  push    573289h
  ret
PlayerMovement_PM_WalkMove_SprintStrafeStub ENDP

PlayerMovement_PM_SetMovementDir_ClampProneLadder_Stub PROC
  push    edx
  mov     edx, PlayerMovement_BGOmnimovement
  test    edx, edx
  jz      stockClamp
  cmp     byte ptr [edx+10h], 0
  jz      stockClamp
  pop     edx

  cmp     eax, 127
  jle     doneWide
  xor     eax, eax
  test    ecx, ecx
  setle   al
  sub     eax, 1
  and     eax, 254
  add     eax, -127
  mov     ecx, eax

doneWide:
  push    443421h
  ret

stockClamp:
  pop     edx

  cmp     eax, 90
  jle     doneStock
  xor     eax, eax
  test    ecx, ecx
  setle   al
  sub     eax, 1
  and     eax, 180
  add     eax, -90
  mov     ecx, eax

doneStock:
  push    443421h
  ret
PlayerMovement_PM_SetMovementDir_ClampProneLadder_Stub ENDP

PlayerMovement_PM_SetMovementDir_ClampGeneric_Stub PROC
  push    edx
  mov     edx, PlayerMovement_BGOmnimovement
  test    edx, edx
  jz      stockClamp
  cmp     byte ptr [edx+10h], 0
  jz      stockClamp
  pop     edx

  cmp     eax, 127
  jle     doneWide
  xor     eax, eax
  test    ecx, ecx
  setle   al
  sub     eax, 1
  and     eax, 254
  add     eax, -127
  mov     ecx, eax

doneWide:
  push    4435C3h
  ret

stockClamp:
  pop     edx

  cmp     eax, 90
  jle     doneStock
  xor     eax, eax
  test    ecx, ecx
  setle   al
  sub     eax, 1
  and     eax, 180
  add     eax, -90
  mov     ecx, eax

doneStock:
  push    4435C3h
  ret
PlayerMovement_PM_SetMovementDir_ClampGeneric_Stub ENDP

PlayerMovement_PM_SetStrafeCondition_Stub PROC
  push    eax
  mov     eax, PlayerMovement_BGOmnimovement
  test    eax, eax
  jz      doStock
  cmp     byte ptr [eax+10h], 0
  jz      doStock

  mov     eax, [esi]
  test    dword ptr [eax+0Ch], 4000h
  jz      doStock
  pop     eax

  mov     eax, [esi]
  push    0
  push    0
  push    7
  push    dword ptr [eax+104h]
  mov     eax, 41FF10h
  call    eax
  add     esp, 10h
  ret

doStock:
  pop     eax

  sub     esp, 0Ch
  movsx   eax, byte ptr [esi+1Fh]
  push    571617h
  ret
PlayerMovement_PM_SetStrafeCondition_Stub ENDP

PlayerMovement_Jump_CheckDive_PerkGate_Stub PROC
  push    eax
  mov     eax, PlayerMovement_BGDive
  test    eax, eax
  jz      doStock
  cmp     byte ptr [eax+10h], 0
  jz      doStock
  pop     eax

  push    56D7C5h
  ret

doStock:
  pop     eax

  test    dword ptr [esi+428h], 100000h
  jnz     perkPresent

  xor     al, al
  add     esp, 6Ch
  ret

perkPresent:
  push    56D7C5h
  ret
PlayerMovement_Jump_CheckDive_PerkGate_Stub ENDP

PlayerMovement_PM_GetMaxSpeed_BackDiagonal_Stub PROC
  push    eax
  mov     eax, PlayerMovement_BGOmnimovement
  test    eax, eax
  jz      doStock
  cmp     byte ptr [eax+10h], 0
  jz      doStock

  mov     eax, [esp+10h]
  test    eax, eax
  jz      doStock
  pop     eax

  push    5738E2h
  ret

doStock:
  pop     eax

  test    al, al
  jge     stockSkip

  mov     eax, dword ptr ds:[7ADC44h]
  fld     dword ptr [eax+10h]
  faddp   st(2), st
  fmulp   st(1), st
  fmul    dword ptr [esp]
  fstp    dword ptr [esp]
  push    5738E6h
  ret

stockSkip:
  push    5738E2h
  ret
PlayerMovement_PM_GetMaxSpeed_BackDiagonal_Stub ENDP

PlayerMovement_PM_GetMaxSpeed_BackPure_Stub PROC
  push    eax
  mov     eax, PlayerMovement_BGOmnimovement
  test    eax, eax
  jz      doStock
  cmp     byte ptr [eax+10h], 0
  jz      doStock

  mov     eax, [esp+10h]
  test    eax, eax
  jz      doStock
  pop     eax

  push    5738E6h
  ret

doStock:
  pop     eax

  test    al, al
  jge     stockSkip

  mov     edx, dword ptr ds:[7ADC44h]
  fld     dword ptr [edx+10h]
  fmul    dword ptr [esp]
  fstp    dword ptr [esp]
  push    5738E6h
  ret

stockSkip:
  push    5738E6h
  ret
PlayerMovement_PM_GetMaxSpeed_BackPure_Stub ENDP

PlayerMovement_Jump_CheckDive_BackDiveVelocity_Stub PROC
  push    eax
  mov     eax, PlayerMovement_BGOmnimovementDive
  test    eax, eax
  jz      doStock
  cmp     byte ptr [eax+10h], 0
  jz      doStock
  pop     eax

  push    56D81Ch
  ret

doStock:
  pop     eax

  fmul    qword ptr ds:[71FE18h]
  sub     esp, 4
  fstp    dword ptr [esp]
  fld     dword ptr [esp]
  add     esp, 4
  push    56D81Ch
  ret
PlayerMovement_Jump_CheckDive_BackDiveVelocity_Stub ENDP

END
