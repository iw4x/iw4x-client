.686
.model flat, C
option casemap:none

EXTERN Weapon_BGWeaponOffHandFix:DWORD

EXTERN Weapon_LoadNoneWeaponHook:PROC

PWF_DISABLE_WEAPON_PICKUP equ 10000h

.code

Weapon_LoadNoneWeaponHookStub PROC
  jmp     Weapon_LoadNoneWeaponHook
Weapon_LoadNoneWeaponHookStub ENDP

Weapon_CG_UpdatePrimaryForAltModeWeapon_Stub PROC
  mov     eax, 440EB0h
  call    eax
  add     esp, 4h

  test    eax, eax
  jz      null

  push    59E349h
  ret

null:
  mov     al, 1
  ret
Weapon_CG_UpdatePrimaryForAltModeWeapon_Stub ENDP

Weapon_CG_SelectWeaponIndex_Stub PROC
  mov     eax, 440EB0h
  call    eax
  add     esp, 4h

  test    eax, eax
  jz      null

  push    48BB2Dh
  ret

null:
  push    48BB1Fh
  ret
Weapon_CG_SelectWeaponIndex_Stub ENDP

Weapon_WeaponEntCanBeGrabbed_Stub PROC
  cmp     dword ptr [esp+8h], 0h
  jz      touched

  push    56E82Ch
  ret

touched:
  test    dword ptr [edi+2BCh], PWF_DISABLE_WEAPON_PICKUP
  jnz     exit_func

  test    eax, eax
  jz      continue_func

exit_func:
  xor     eax, eax
  ret

continue_func:
  push    56E84Ch
  ret
Weapon_WeaponEntCanBeGrabbed_Stub ENDP

Weapon_CG_DrawItem_Stub PROC
  mov     eax, 440EB0h
  call    eax

  test    eax, eax
  jz      null

  push    5860B1h
  ret

null:
  add     esp, 4h

  push    58615Ah
  ret
Weapon_CG_DrawItem_Stub ENDP

Weapon_CG_CreateViewModelTree_Stub PROC
  cmp     dword ptr [esp+4h], 0
  jnz     create

  mov     eax, [esi]
  mov     [esp+4h], eax

create:
  mov     eax, 4CE030h
  jmp     eax
Weapon_CG_CreateViewModelTree_Stub ENDP

Weapon_JavelinResetHook_Stub PROC
  call    dword ptr [gamePM_Weapon_OffHandEnd_t]

  push    eax
  mov     eax, Weapon_BGWeaponOffHandFix
  cmp     byte ptr [eax+10h], 1
  pop     eax

  jne     safeReturn

  mov     dword ptr [esi+34h], 0

safeReturn:
  pop     edi
  pop     esi
  pop     ebx
  ret

gamePM_Weapon_OffHandEnd_t:
  dd      577A10h
Weapon_JavelinResetHook_Stub ENDP

END
