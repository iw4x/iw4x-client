.686
.model flat, C
option casemap:none

EXTERN Chat_SendChat:BYTE

EXTERN Chat_EvaluateSay:PROC
EXTERN Chat_CG_AddToTeamChat:PROC
EXTERN PlayerName_CleanStrStub:PROC

.code

Chat_PreSayStub PROC
  mov     eax, [esp+100h+10h]

  push    eax
  pushad

  push    dword ptr [esp+100h+30h]
  push    dword ptr [esp+100h+2Ch]
  push    eax
  call    Chat_EvaluateSay
  add     esp, 0Ch

  mov     [esp+20h], eax
  popad
  pop     eax

  mov     [esp+100h+10h], eax

  jmp     PlayerName_CleanStrStub
Chat_PreSayStub ENDP

Chat_PostSayStub PROC
  push    eax

  xor     eax, eax
  mov     al, Chat_SendChat

  test    al, al
  jnz     return

  pop     eax
  ret

return:
  pop     eax

  push    5DF620h
  ret
Chat_PostSayStub ENDP

Chat_CG_AddToTeamChat_Stub PROC
  pushad

  push    ecx
  call    Chat_CG_AddToTeamChat
  add     esp, 4h

  popad
  ret
Chat_CG_AddToTeamChat_Stub ENDP

END
