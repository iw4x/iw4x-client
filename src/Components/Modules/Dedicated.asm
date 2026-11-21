option casemap:none

EXTERN Dedicated_Com_ClampMsec:PROC

.code

Dedicated_Com_ClampMsec_Stub PROC
  push    rax
  push    rcx
  sub     rsp, 20h

  call    Dedicated_Com_ClampMsec

  add     rsp, 20h
  pop     rcx
  pop     rax

  mov     rdx, 141BD7958h
  mov     [rdx], eax

  mov     rdx, 1401F4082h
  jmp     rdx
Dedicated_Com_ClampMsec_Stub ENDP

END
