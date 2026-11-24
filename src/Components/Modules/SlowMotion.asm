option casemap:none

EXTERN SlowMotion_Com_UpdateSlowMotion:PROC

.code

SlowMotion_Com_UpdateSlowMotion_Stub PROC
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

  mov     ecx, r9d
  call    SlowMotion_Com_UpdateSlowMotion
  test    al, al

  movdqu  xmm0, [rsp+20h]
  movdqu  xmm1, [rsp+30h]
  movdqu  xmm2, [rsp+40h]
  movdqu  xmm3, [rsp+50h]
  movdqu  xmm4, [rsp+60h]
  movdqu  xmm5, [rsp+70h]
  lea     rsp, [rsp+88h]
  pop     r11
  pop     r10
  pop     r9
  pop     r8
  pop     rdx
  pop     rcx
  pop     rax
  jnz     skip

  mov     rax, 141BD7960h
  cmp     byte ptr [rax], 0
  mov     rax, 1401F42D3h
  jz      off
  mov     rax, 1401F425Ch
off:
  jmp     rax

skip:
  mov     rax, 1401F42D3h
  jmp     rax
SlowMotion_Com_UpdateSlowMotion_Stub ENDP

END
