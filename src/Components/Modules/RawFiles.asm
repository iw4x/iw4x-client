EXTERN RawFiles_GetMenuBuffer:PROC

.code

RawFiles_GetMenuBufferStub PROC
  sub     rsp, 20h
  mov     rcx, rax
  call    RawFiles_GetMenuBuffer
  add     rsp, 20h

  mov     rbx, rax
  mov     [rsp+68h], rax

  test    rax, rax
  jz      missing

  mov     rcx, 14026D645h
  jmp     rcx

missing:
  mov     rcx, 14026D5F1h
  jmp     rcx
RawFiles_GetMenuBufferStub ENDP

END
