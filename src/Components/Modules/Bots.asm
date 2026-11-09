.686
.model flat, C
option casemap:none

EXTERN Bots_BotAiAction:PROC
EXTERN Bots_G_SelectWeaponIndex:PROC

.code

Bots_SV_BotUserMove_Hk PROC
  pushad

  push    edi
  call    Bots_BotAiAction
  add     esp, 4

  popad
  ret
Bots_SV_BotUserMove_Hk ENDP

Bots_G_SelectWeaponIndex_Hk PROC
  pushad

  push    dword ptr [esp+20h+8h]
  push    dword ptr [esp+20h+8h]
  call    Bots_G_SelectWeaponIndex
  add     esp, 8h

  popad

  mov     eax, [esp+8h]
  push    eax

  push    441B85h
  ret
Bots_G_SelectWeaponIndex_Hk ENDP

END
