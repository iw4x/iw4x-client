option casemap:none

EXTERN Chat_SendChat:BYTE

EXTERN Chat_EvaluateSay:PROC

.code

Chat_PreSayStub1 PROC
  push    rcx
  sub     rsp, 20h

  mov     rcx, rsi
  mov     rdx, r14
  mov     r8d, edi
  call    Chat_EvaluateSay

  add     rsp, 20h
  pop     rcx

  mov     rax, 14028A4E0h
  jmp     rax
Chat_PreSayStub1 ENDP

Chat_PreSayStub2 PROC
  push    rcx
  sub     rsp, 20h

  mov     rcx, r15
  mov     rdx, r14
  mov     r8d, edi
  call    Chat_EvaluateSay

  add     rsp, 20h
  pop     rcx

  mov     rax, 14028A4E0h
  jmp     rax
Chat_PreSayStub2 ENDP

Chat_PostSayStub1 PROC
  cmp     Chat_SendChat, 0
  jz      skip

  mov     rax, 141865260h
  cmp     dword ptr [rax], r13d

  mov     rax, 14019968Ah
  jmp     rax

skip:
  mov     rax, 1401998C7h
  jmp     rax
Chat_PostSayStub1 ENDP

Chat_PostSayStub2 PROC
  cmp     Chat_SendChat, 0
  jz      skip

  mov     rax, 140199D00h
  jmp     rax

skip:
  ret
Chat_PostSayStub2 ENDP

END
