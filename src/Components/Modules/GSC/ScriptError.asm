option casemap:none

EXTERN ScriptError_VM_Error:PROC

.data

ScriptError_VM_ErrorReturn DQ 14022C486h

.code

ScriptError_VM_ErrorStub PROC
  push    rax
  push    rcx
  push    rdx
  push    r8
  push    r9
  push    r10
  push    r11
  sub     rsp, 88h

  movdqu  [rsp + 20h], xmm0
  movdqu  [rsp + 30h], xmm1
  movdqu  [rsp + 40h], xmm2
  movdqu  [rsp + 50h], xmm3
  movdqu  [rsp + 60h], xmm4
  movdqu  [rsp + 70h], xmm5

  mov     rcx, rbx
  call    ScriptError_VM_Error

  movdqu  xmm0, [rsp + 20h]
  movdqu  xmm1, [rsp + 30h]
  movdqu  xmm2, [rsp + 40h]
  movdqu  xmm3, [rsp + 50h]
  movdqu  xmm4, [rsp + 60h]
  movdqu  xmm5, [rsp + 70h]

  add     rsp, 88h
  pop     r11
  pop     r10
  pop     r9
  pop     r8
  pop     rdx
  pop     rcx
  pop     rax

  xor     esi, esi
  mov     rdi, 142283E50h
  jmp     QWORD PTR [ScriptError_VM_ErrorReturn]
ScriptError_VM_ErrorStub ENDP

END
