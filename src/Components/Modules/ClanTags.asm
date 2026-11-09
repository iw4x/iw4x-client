.686
.model flat, C
option casemap:none

EXTERN ClanTags_ClientUserinfoChanged:PROC
EXTERN ClanTags_GetClanTagWithName:PROC
EXTERN ClanTags_PlayerCards_SetCachedPlayerData:PROC

.code

ClanTags_ClientUserinfoChanged_Stub PROC
  pushad

  push    dword ptr [esp+20h+824h]
  push    ecx
  call    ClanTags_ClientUserinfoChanged
  add     esp, 8h

  popad

  push    445334h
  push    47C820h
  ret
ClanTags_ClientUserinfoChanged_Stub ENDP

ClanTags_DrawPlayerNameOnScoreboard PROC
  push    eax
  pushad

  push    edi
  push    dword ptr [ebp]

  call    ClanTags_GetClanTagWithName
  add     esp, 8h

  mov     [esp+20h], eax

  popad
  pop     edi

  push    591247h
  push    5909E0h
  ret
ClanTags_DrawPlayerNameOnScoreboard ENDP

ClanTags_PlayerCards_SetCachedPlayerData_Stub PROC
  call    dword ptr [gameI_strncpyz]
  add     esp, 0Ch

  mov     byte ptr [esi+3Ch], 0h

  push    dword ptr [esp+0Ch]
  push    esi
  call    ClanTags_PlayerCards_SetCachedPlayerData
  add     esp, 8h

  pop     esi
  ret

gameI_strncpyz:
  dd      4D6F80h
ClanTags_PlayerCards_SetCachedPlayerData_Stub ENDP

END
