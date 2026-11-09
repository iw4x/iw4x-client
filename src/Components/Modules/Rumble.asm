.686
.model flat, C
option casemap:none

EXTERN Rumble_CG_FireWeapon_Rumble:PROC
EXTERN Rumble_PlayNoteMappedRumbleAliases:PROC
EXTERN Rumble_CG_EntityEvents_Hk:PROC
EXTERN Rumble_MeleeRumble_Hook:PROC

.code

Rumble_CG_FireWeapon_FireSoundHk PROC
  pushad

  push    ebx
  push    dword ptr [esp+34h]
  push    esi
  push    dword ptr [esp+30h]

  call    Rumble_CG_FireWeapon_Rumble

  add     esp, 4h*4

  popad

  sub     esp, 10h
  push    ebp
  mov     ebp, [esp+24h]

  push    59D7D8h
  ret
Rumble_CG_FireWeapon_FireSoundHk ENDP

Rumble_PlayNoteMappedSoundAliases_Stub PROC
  pushad
  push    edi
  push    ecx
  push    edx

  call    Rumble_PlayNoteMappedRumbleAliases

  pop     edx
  pop     ecx
  pop     edi
  popad

  mov     eax, [edi+18h]
  cmp     word ptr [eax], 0

  push    59C447h
  ret
Rumble_PlayNoteMappedSoundAliases_Stub ENDP

Rumble_CG_EntityEvents_Stub PROC
  push    eax
  pushad

  push    ebx
  push    dword ptr [esp+0A8h+20h+4h]

  call    Rumble_CG_EntityEvents_Hk

  add     esp, 8
  mov     [esp+20h], eax

  popad
  pop     eax

  test    al, al
  jz      processCgEvents

  push    4DED0Ah
  ret

processCgEvents:
  mov     edx, dword ptr ds:[9F5CE4h]

  push    4DCF8Ah
  ret
Rumble_CG_EntityEvents_Stub ENDP

Rumble_MeleeRumble_Stub PROC
  pushad

  push    ebx
  push    esi
  call    Rumble_MeleeRumble_Hook
  add     esp, 8

  popad

  cmp     dword ptr [esi+158h], 0
  je      goBack

  push    05FCD7Dh
  ret

goBack:
  push    5FCDA0h
  ret
Rumble_MeleeRumble_Stub ENDP

END
