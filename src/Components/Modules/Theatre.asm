EXTERN Theatre_StoreBaseline:PROC
EXTERN Theatre_AdjustTimeDelta:PROC

.code

Theatre_BaselineStoreStub PROC
  mov     rax, 140C397BCh
  mov     dword ptr [rax], ebx

  sub     rsp, 20h
  mov     rcx, rbp
  call    Theatre_StoreBaseline
  add     rsp, 20h

  mov     rax, 140100FF6h
  jmp     rax
Theatre_BaselineStoreStub ENDP

Theatre_AdjustTimeDeltaStub PROC
  mov     rax, 140C397B4h
  cmp     dword ptr [rax], 0
  jnz     demo

  mov     r11d, r9d
  mov     rax, 1406CFE30h
  mov     dword ptr [rax], r15d
  mov     rax, 1400F51ABh
  jmp     rax

demo:
  sub     rsp, 20h
  call    Theatre_AdjustTimeDelta
  add     rsp, 20h

  mov     rax, 1400F52D6h
  jmp     rax
Theatre_AdjustTimeDeltaStub ENDP

Theatre_UISetActiveMenuStub PROC
  mov     rax, 140C397B4h
  cmp     dword ptr [rax], 0
  jz      menu

  mov     rax, 140270E71h
  jmp     rax

menu:
  mov     edx, 10h
  mov     rax, 140270E8Bh
  jmp     rax
Theatre_UISetActiveMenuStub ENDP

END
