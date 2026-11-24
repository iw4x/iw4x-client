EXTERN Threading_SVFrameWaitFunc:PROC
EXTERN Threading_ComFrameWait:PROC

.code

Threading_SVFrameWaitStub PROC
  sub     rsp, 20h
  call    Threading_SVFrameWaitFunc
  add     rsp, 20h

  mov     rax, 141BD7948h
  mov     rax, [rax]

  mov     rcx, 14023AA9Bh
  jmp     rcx
Threading_SVFrameWaitStub ENDP

Threading_ComFrameWaitStub PROC
  sub     rsp, 20h
  mov     ecx, ebx
  call    Threading_ComFrameWait
  add     rsp, 20h

  mov     ecx, eax

  mov     rax, 1401F4082h
  jmp     rax
Threading_ComFrameWaitStub ENDP

END
