option casemap:none

EXTERN Bots_BotAiAction:PROC

.code

Bots_SV_BotUserMoveStub PROC
  push    rcx
  sub     rsp, 28h

  lea     rcx, [rsi-212B8h]
  call    Bots_BotAiAction

  add     rsp, 28h
  pop     rcx

  test    al, al
  jnz     handled

  xor     eax, eax
  xorps   xmm0, xmm0

  mov     rdx, 14023BC46h
  jmp     rdx

handled:
  mov     rax, 14023BE5Dh
  jmp     rax
Bots_SV_BotUserMoveStub ENDP

END
