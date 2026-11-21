option casemap:none

EXTERN ScriptExtension_ReplacedPos:QWORD

EXTERN ScriptExtension_GetReplacedPos:PROC

.code

ScriptExtension_VMExecuteInternalStub PROC
  push    rax
  push    rcx
  push    rdx
  push    r8
  push    r9
  push    r10
  push    r11
  sub     rsp, 88h
  movdqu  [rsp+20h], xmm0
  movdqu  [rsp+30h], xmm1
  movdqu  [rsp+40h], xmm2
  movdqu  [rsp+50h], xmm3
  movdqu  [rsp+60h], xmm4
  movdqu  [rsp+70h], xmm5

  call    ScriptExtension_GetReplacedPos

  movdqu  xmm0, [rsp+20h]
  movdqu  xmm1, [rsp+30h]
  movdqu  xmm2, [rsp+40h]
  movdqu  xmm3, [rsp+50h]
  movdqu  xmm4, [rsp+60h]
  movdqu  xmm5, [rsp+70h]
  add     rsp, 88h
  pop     r11
  pop     r10
  pop     r9
  pop     r8
  pop     rdx
  pop     rcx
  pop     rax

  cmp     ScriptExtension_ReplacedPos, 0
  jne     SetPos

Loc0:
  movzx   r8d, byte ptr [rcx]
  lea     rdx, [rcx+1]

  push    rax

  mov     rax, 142287F98h
  mov     [rax], r8d

  mov     rax, 142287F78h
  mov     [rax], rdx

  pop     rax

  cmp     r8d, 8Bh
  jmp     qword ptr [resume]

SetPos:
  mov     rcx, ScriptExtension_ReplacedPos
  mov     ScriptExtension_ReplacedPos, 0

  jmp     Loc0

resume:
  dq      140229FC8h
ScriptExtension_VMExecuteInternalStub ENDP

END
