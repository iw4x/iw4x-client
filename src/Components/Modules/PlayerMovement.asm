option casemap:none

EXTERN PlayerMovement_BGBounces:QWORD
EXTERN PlayerMovement_BGBouncesAllAngles:QWORD
EXTERN PlayerMovement_BGDisableLandingSlowdown:QWORD
EXTERN PlayerMovement_BGBunnyHopAuto:QWORD
EXTERN PlayerMovement_BGLadderFixedInput:QWORD
EXTERN PlayerMovement_BGSprintIgnoreRepress:QWORD
EXTERN PlayerMovement_BGOmnimovement:QWORD
EXTERN PlayerMovement_BGDive:QWORD
EXTERN PlayerMovement_BGOmnimovementDive:QWORD

.code

PlayerMovement_PM_StepSlideMoveStub PROC
  mov     rax, PlayerMovement_BGBounces
  test    rax, rax
  jz      stock
  cmp     dword ptr [rax+10h], 0
  jle     stock

  mov     rax, PlayerMovement_BGBouncesAllAngles
  test    rax, rax
  jz      regularBounce
  cmp     dword ptr [rax+10h], 2
  jne     regularBounce

  mov     rax, 140095FBEh
  jmp     rax

regularBounce:
  mov     rax, 140095F7Bh
  jmp     rax

stock:
  test    r13d, r13d
  jnz     noBounce

  mov     rax, 140095F7Bh
  jmp     rax

noBounce:
  mov     rax, 140095F89h
  jmp     rax
PlayerMovement_PM_StepSlideMoveStub ENDP

PlayerMovement_PM_ProjectVelocityStub PROC
  mov     rax, PlayerMovement_BGBouncesAllAngles
  test    rax, rax
  jz      stock
  cmp     dword ptr [rax+10h], 0
  je      stock

  mov     rax, 140091C29h
  jmp     rax

stock:
  comiss  xmm10, xmm9
  jbe     noBounce

  mov     rax, 140091C29h
  jmp     rax

noBounce:
  mov     rax, 140091C44h
  jmp     rax
PlayerMovement_PM_ProjectVelocityStub ENDP

PlayerMovement_PM_CrashLand_Stub1 PROC
  mov     rax, PlayerMovement_BGDisableLandingSlowdown
  test    rax, rax
  jz      stock
  cmp     byte ptr [rax+10h], 0
  je      stock

  mov     rax, 14008FF08h
  jmp     rax

stock:
  movss   xmm0, dword ptr [rdi+28h]
  mov     rax, 14008FEDBh
  jmp     rax
PlayerMovement_PM_CrashLand_Stub1 ENDP

PlayerMovement_PM_CrashLand_Stub2 PROC
  mov     rax, PlayerMovement_BGDisableLandingSlowdown
  test    rax, rax
  jz      stock
  cmp     byte ptr [rax+10h], 0
  je      stock

  mov     rax, 14008FFAAh
  jmp     rax

stock:
  movss   xmm0, dword ptr [rdi+28h]
  mov     rax, 14008FF7Dh
  jmp     rax
PlayerMovement_PM_CrashLand_Stub2 ENDP

PlayerMovement_Jump_Check_Stub PROC
  mov     rcx, PlayerMovement_BGBunnyHopAuto
  test    rcx, rcx
  jz      stock
  cmp     byte ptr [rcx+10h], 1
  je      autoHop

stock:
  test    dword ptr [rdi+34h], 400h
  jz      autoHop

  mov     rcx, 140087340h
  jmp     rcx

autoHop:
  mov     rcx, 14008735Eh
  jmp     rcx
PlayerMovement_Jump_Check_Stub ENDP

PlayerMovement_PM_LadderMove_PitchStub PROC
  mov     rax, PlayerMovement_BGLadderFixedInput
  test    rax, rax
  jz      normalize
  cmp     byte ptr [rax+10h], 1
  jne     normalize

  mov     eax, 3F800000h
  movd    xmm7, eax

normalize:
  mov     rax, 14001ED50h
  jmp     rax
PlayerMovement_PM_LadderMove_PitchStub ENDP

PlayerMovement_PM_UpdateSprint_RepressCallStub PROC
  mov     rax, PlayerMovement_BGSprintIgnoreRepress
  test    rax, rax
  jz      stock
  cmp     byte ptr [rax+10h], 1
  jne     stock

  mov     rax, 140092A16h
  jmp     rax

stock:
  mov     r8, rsi
  mov     rdx, rdi
  mov     rax, 1400927F0h
  jmp     rax
PlayerMovement_PM_UpdateSprint_RepressCallStub ENDP

PlayerMovement_CL_KeyMove_SprintBit_Stub PROC
  mov     rax, PlayerMovement_BGOmnimovement
  test    rax, rax
  jz      stock
  cmp     byte ptr [rax+10h], 0
  je      stock

  mov     rax, 1400F6F4Bh
  jmp     rax

stock:
  cmp     byte ptr [r10+4Ch], 0
  jnz     stockSkip

  mov     rax, 1400F6F4Bh
  jmp     rax

stockSkip:
  mov     rax, 1400F6F71h
  jmp     rax
PlayerMovement_CL_KeyMove_SprintBit_Stub ENDP

SprintIntent MACRO back
  LOCAL compare
  movsx   ecx, byte ptr [rdi+22h]

  mov     r10, PlayerMovement_BGOmnimovement
  test    r10, r10
  jz      compare
  cmp     byte ptr [r10+10h], 0
  je      compare

  mov     r10d, ecx
  neg     r10d
  cmovl   r10d, ecx

  movsx   r11d, byte ptr [rdi+23h]
  mov     ecx, r11d
  neg     ecx
  cmovl   ecx, r11d

  cmp     ecx, r10d
  cmovl   ecx, r10d

compare:
  cmp     ecx, [rax+10h]
  mov     r11, back
  jmp     r11
ENDM

PlayerMovement_PM_SprintStartInterferingButtons_Stub PROC
  SprintIntent 14009288Ah
PlayerMovement_PM_SprintStartInterferingButtons_Stub ENDP

PlayerMovement_PM_SprintEndingButtons_Stub PROC
  SprintIntent 1400927A5h
PlayerMovement_PM_SprintEndingButtons_Stub ENDP

PlayerMovement_PM_WalkMove_SprintStrafeStub PROC
  mov     rax, PlayerMovement_BGOmnimovement
  test    rax, rax
  jz      stock
  cmp     byte ptr [rax+10h], 0
  je      stock

  mov     rax, 140093D23h
  jmp     rax

stock:
  movsx   eax, byte ptr [rsi+23h]
  movd    xmm0, eax
  mov     rax, 140093D0Dh
  jmp     rax
PlayerMovement_PM_WalkMove_SprintStrafeStub ENDP

PlayerMovement_PM_SetMovementDir_ClampStub PROC
  mov     edx, 90

  mov     r8, PlayerMovement_BGOmnimovement
  test    r8, r8
  jz      clamp
  cmp     byte ptr [r8+10h], 0
  je      clamp
  mov     edx, 127

clamp:
  cmp     eax, edx
  jle     done

  test    ecx, ecx
  jg      positive
  neg     edx

positive:
  mov     ecx, edx

done:
  mov     rax, 140091E2Ch
  jmp     rax
PlayerMovement_PM_SetMovementDir_ClampStub ENDP

PlayerMovement_PM_SetStrafeCondition_Stub PROC
  mov     r10, PlayerMovement_BGOmnimovement
  test    r10, r10
  jz      done
  cmp     byte ptr [r10+10h], 0
  je      done

  mov     r10, [rsi]
  test    dword ptr [r10+0Ch], 4000h
  jz      done

  xor     r8d, r8d

done:
  mov     r10, 1400862C0h
  jmp     r10
PlayerMovement_PM_SetStrafeCondition_Stub ENDP

PlayerMovement_Jump_CheckDive_PerkGate_Stub PROC
  mov     rax, PlayerMovement_BGDive
  test    rax, rax
  jz      stock
  cmp     byte ptr [rax+10h], 0
  je      stock

  mov     rax, 140087742h
  jmp     rax

stock:
  xor     dil, dil
  mov     rax, 1400878D8h
  jmp     rax
PlayerMovement_Jump_CheckDive_PerkGate_Stub ENDP

GetMaxSpeedBack MACRO back
  LOCAL stock
  mov     rax, PlayerMovement_BGOmnimovement
  test    rax, rax
  jz      stock
  cmp     byte ptr [rax+10h], 0
  je      stock
  test    ebp, ebp
  jz      stock

  mov     rax, 140090665h
  jmp     rax

stock:
  mov     rax, 14043EA90h
  mov     rax, [rax]
  mov     rcx, back
  jmp     rcx
ENDM

PlayerMovement_PM_GetMaxSpeed_BackDiagonal_Stub PROC
  GetMaxSpeedBack 140090614h
PlayerMovement_PM_GetMaxSpeed_BackDiagonal_Stub ENDP

PlayerMovement_PM_GetMaxSpeed_BackPure_Stub PROC
  GetMaxSpeedBack 140090632h
PlayerMovement_PM_GetMaxSpeed_BackPure_Stub ENDP

PlayerMovement_Jump_CheckDive_BackDiveVelocity_Stub PROC
  mov     rax, PlayerMovement_BGOmnimovementDive
  test    rax, rax
  jz      stock
  cmp     byte ptr [rax+10h], 0
  jne     done

stock:
  mov     rax, 140363E5Ch
  mulss   xmm2, dword ptr [rax]

done:
  mov     rax, 140087790h
  jmp     rax
PlayerMovement_Jump_CheckDive_BackDiveVelocity_Stub ENDP

END
