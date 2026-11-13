option casemap:none

EXTERN Logger_RedirectOSPath:PROC

.code

Logger_BuildOSPath_Stub PROC
  sub     rsp, 20h
  mov     rcx, r13
  mov     rdx, r12
  call    Logger_RedirectOSPath
  add     rsp, 20h

  mov     rcx, rbx
  mov     rdx, r12
  mov     r8, r13
  mov     r9, r14
  mov     rsi, 0FFFFFFFFFFFFFFFFh

  mov     rax, 140273B20h
  jmp     rax
Logger_BuildOSPath_Stub ENDP

END
