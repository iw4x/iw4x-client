option casemap:none

EXTERN Network_CL_HandleCommand:PROC

.code

Network_CL_HandleCommandStub PROC
  sub     rsp, 20h
  mov     rcx, rdi
  mov     rdx, rsi
  mov     r8, r12
  call    Network_CL_HandleCommand
  add     rsp, 20h

  test    al, al
  jz      unhandled

  mov     rax, 1400F96F3h
  jmp     rax

unhandled:
  mov     rdx, 140377D00h
  mov     rax, 1400F9E13h
  jmp     rax
Network_CL_HandleCommandStub ENDP

END
