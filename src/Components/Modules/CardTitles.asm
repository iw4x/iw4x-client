option casemap:none

EXTERN CardTitles_GetPlayerCardClientInfo:PROC

.code

CardTitles_GetPlayerCardClientInfoStub PROC
  sub     rsp, 20h
  mov     ecx, [rbx+4]
  mov     rdx, rbx
  call    CardTitles_GetPlayerCardClientInfo
  add     rsp, 20h

  mov     [rdi+8], eax

  mov     rax, 140250136h
  jmp     rax
CardTitles_GetPlayerCardClientInfoStub ENDP

END
