option casemap:none

EXTERN ClientCommand_GiveAll_Hk:PROC

.code

ClientCommand_GiveAll_Stub PROC
  pushfq
  push    r15
  push    r14
  push    r13
  push    r12
  push    r11
  push    r10
  push    r9
  push    r8
  push    rdi
  push    rsi
  push    rbp
  push    rsp
  push    rbx
  push    rdx
  push    rcx
  push    rax

  mov     rcx, rsp
  mov     rbx, rsp
  and     rsp, -16
  sub     rsp, 80h
  movdqa  [rsp+20h], xmm0
  movdqa  [rsp+30h], xmm1
  movdqa  [rsp+40h], xmm2
  movdqa  [rsp+50h], xmm3
  movdqa  [rsp+60h], xmm4
  movdqa  [rsp+70h], xmm5

  call    ClientCommand_GiveAll_Hk

  movdqa  xmm0, [rsp+20h]
  movdqa  xmm1, [rsp+30h]
  movdqa  xmm2, [rsp+40h]
  movdqa  xmm3, [rsp+50h]
  movdqa  xmm4, [rsp+60h]
  movdqa  xmm5, [rsp+70h]
  mov     rsp, rbx

  pop     rax
  pop     rcx
  pop     rdx
  pop     rbx
  add     rsp, 8
  pop     rbp
  pop     rsi
  pop     rdi
  pop     r8
  pop     r9
  pop     r10
  pop     r11
  pop     r12
  pop     r13
  pop     r14
  pop     r15
  popfq
  ret
ClientCommand_GiveAll_Stub ENDP

END
