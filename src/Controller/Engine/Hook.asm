option casemap:none

EXTERN Controller_ShouldUse:PROC
EXTERN Controller_ReadDeltaMovement:PROC
EXTERN Controller_LocalizedKeyNames:PROC

.code

Controller_PlayerUseEntityStub PROC
  cmp     edx, [rax+10h]
  jl      skip

  push    r8
  push    r9
  sub     rsp, 20h

  mov     rcx, rbx
  call    Controller_ShouldUse

  add     rsp, 20h
  pop     r9
  pop     r8

  test    al, al
  jz      skip

  mov     rax, 14018BE46h
  jmp     rax

skip:
  mov     rax, 14018BE5Fh
  jmp     rax
Controller_PlayerUseEntityStub ENDP

Controller_WriteDeltaMovementStub PROC
  mov     [rsp+50h], r15
  mov     ebx, 2

  movzx   r15d, word ptr [rsi+1Ah]
  movzx   r12d, word ptr [rbp+1Ah]

  mov     rax, 1402045E1h
  jmp     rax
Controller_WriteDeltaMovementStub ENDP

Controller_ReadDeltaMovementStub1 PROC
  sub     rsp, 20h

  mov     rcx, rsi
  mov     edx, ebp
  mov     r8, r14
  mov     r9, rdi
  call    Controller_ReadDeltaMovement

  add     rsp, 20h

  mov     rax, 140203B1Dh
  jmp     rax
Controller_ReadDeltaMovementStub1 ENDP

Controller_ReadDeltaMovementStub2 PROC
  sub     rsp, 20h

  mov     rcx, rsi
  mov     edx, ebp
  mov     r8, r14
  mov     r9, rdi
  call    Controller_ReadDeltaMovement

  add     rsp, 20h

  mov     rax, 140203C21h
  jmp     rax
Controller_ReadDeltaMovementStub2 ENDP

Controller_LocalizedKeyNamesStub PROC
  pushfq
  push    rcx
  push    rdx
  push    r8
  push    r9
  push    r10
  push    r11
  sub     rsp, 20h

  call    Controller_LocalizedKeyNames

  add     rsp, 20h
  pop     r11
  pop     r10
  pop     r9
  pop     r8
  pop     rdx
  pop     rcx
  popfq
  ret
Controller_LocalizedKeyNamesStub ENDP

END
