option casemap:none

EXTERN ClanTags_ClientUserinfoChanged:PROC
EXTERN ClanTags_GetClanTagWithName:PROC

.code

ClanTags_ClientUserinfoChanged_Stub PROC
  push    rcx
  push    rdx
  sub     rsp, 28h

  mov     edx, ebx
  call    ClanTags_ClientUserinfoChanged

  add     rsp, 28h
  pop     rdx
  pop     rcx

  mov     rax, 14028AF50h
  jmp     rax
ClanTags_ClientUserinfoChanged_Stub ENDP

ClanTags_ClientConnectUserinfo_Stub PROC
  push    rcx
  push    rdx
  sub     rsp, 28h

  mov     edx, r12d
  call    ClanTags_ClientUserinfoChanged

  add     rsp, 28h
  pop     rdx
  pop     rcx

  mov     rax, 14028AF50h
  jmp     rax
ClanTags_ClientConnectUserinfo_Stub ENDP

ClanTags_DrawPlayerNameOnScoreboard PROC
  sub     rsp, 20h

  mov     ecx, [r15]
  lea     rdx, [rbx+0Ch]
  call    ClanTags_GetClanTagWithName

  add     rsp, 20h

  mov     rdx, rax
  mov     rax, 1400E3DC1h
  jmp     rax
ClanTags_DrawPlayerNameOnScoreboard ENDP

END
