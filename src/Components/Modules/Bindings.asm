option casemap:none

EXTERN Bindings_KeyUp:PROC

.code

Bindings_KeyUpStub PROC
  cmp     eax, 41h
  jge     ours

  mov     r11, 1400EEA1Ah
  jmp     r11

ours:
  sub     rsp, 20h
  mov     r8d, ebx
  mov     edx, eax
  mov     ecx, edi
  call    Bindings_KeyUp
  add     rsp, 20h

  mov     rax, 1406CCB70h
  mov     rdx, qword ptr [rax]

  mov     r11, 1400EEA32h
  jmp     r11
Bindings_KeyUpStub ENDP

END
