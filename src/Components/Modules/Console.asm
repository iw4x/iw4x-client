option casemap:none

EXTERN Console_MakeRoomForText:PROC

.code

Console_Sys_PrintStub PROC
  sub     rsp, 20h
  mov     ecx, ebx
  call    Console_MakeRoomForText
  add     rsp, 20h

  mov     rax, 14678B738h
  mov     rcx, [rax]
  mov     edx, 0B1h

  mov     rax, 1402A784Eh
  jmp     rax
Console_Sys_PrintStub ENDP

END
