option casemap:none

EXTERN Weapon_BGWeaponOffHandFix:QWORD

PWF_DISABLE_WEAPON_PICKUP equ 10000h

.code

Weapon_CG_SelectWeaponIndex_Stub PROC
  sub     rsp, 28h
  mov     rax, 14009C880h
  call    rax
  add     rsp, 28h

  test    rax, rax
  jz      null

  ret

null:
  add     rsp, 8h
  mov     rax, 1400C52F1h
  jmp     rax
Weapon_CG_SelectWeaponIndex_Stub ENDP

Weapon_WeaponEntCanBeGrabbed_Stub PROC
  test    r14d, r14d
  jz      touched

  mov     rcx, 14008D773h
  jmp     rcx

touched:
  test    dword ptr [rdi+2BCh], PWF_DISABLE_WEAPON_PICKUP
  jnz     exit_func

  mov     rcx, 14008D7A1h
  jmp     rcx

exit_func:
  mov     rcx, 14008D706h
  jmp     rcx
Weapon_WeaponEntCanBeGrabbed_Stub ENDP

Weapon_CG_DrawItem_Stub PROC
  sub     rsp, 28h
  mov     rax, 14009C880h
  call    rax
  add     rsp, 28h

  test    rax, rax
  jz      null

  ret

null:
  add     rsp, 8h
  mov     rax, 1400D5428h
  jmp     rax
Weapon_CG_DrawItem_Stub ENDP

Weapon_CG_CreateViewModelTree_Stub PROC
  test    rcx, rcx
  jnz     create

  mov     rcx, [rax+rsi*8]

create:
  mov     rax, 1401F6860h
  jmp     rax
Weapon_CG_CreateViewModelTree_Stub ENDP

Weapon_JavelinResetHook_Stub PROC
  xor     edi, edi

  mov     rax, Weapon_BGWeaponOffHandFix
  cmp     byte ptr [rax+10h], 1
  jne     safeReturn

  mov     dword ptr [rbx+34h], edi

safeReturn:
  ret
Weapon_JavelinResetHook_Stub ENDP

END
