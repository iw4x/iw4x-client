option casemap:none

EXTERN Console_MakeRoomForText:PROC
EXTERN Console_CL_ConsoleKeyEvent:PROC
EXTERN Console_Con_CancelAutoComplete:PROC
EXTERN Console_Console_Key:PROC

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

Console_CL_ConsolePrintStub PROC
  mov     ecx, r12d
  mov     edx, r13d
  mov     r8d, 1
  mov     r9d, r14d
  mov     rax, 1400EE410h
  call    rax

  mov     rax, 1400EBCBDh
  jmp     rax
Console_CL_ConsolePrintStub ENDP

Console_CL_KeyEventStub PROC
  push    r8
  push    r10
  sub     rsp, 20h
  mov     ecx, edi
  mov     edx, ebx
  mov     r8d, esi
  call    Console_CL_ConsoleKeyEvent
  add     rsp, 20h
  pop     r10
  pop     r8

  test    al, al
  jz      continue

  mov     rax, 1400EEAE2h
  jmp     rax

continue:
  mov     rax, 1406CCB70h
  mov     rdx, [rax]
  test    dl, 8
  jz      no_location

  mov     rax, 1400EE897h
  jmp     rax

no_location:
  mov     rax, 1400EE8CFh
  jmp     rax
Console_CL_KeyEventStub ENDP

Console_CL_KeyEventEscapeStub PROC
  test    dl, 1
  jz      no_console

  push    rdx
  push    r8
  sub     rsp, 20h
  call    Console_Con_CancelAutoComplete
  add     rsp, 20h
  pop     r8
  pop     rdx

no_console:
  test    dl, 10h
  jnz     ui

  mov     rax, 1400EE962h
  jmp     rax

ui:
  mov     rax, 1400EE9C4h
  jmp     rax
Console_CL_KeyEventEscapeStub ENDP

Console_CL_KeyEventDownStub PROC
  test    dl, 1
  jnz     console

  mov     rax, 1400EEA67h
  jmp     rax

console:
  mov     ecx, edi
  mov     edx, ebx
  call    Console_Console_Key

  mov     rax, 1400EEADDh
  jmp     rax
Console_CL_KeyEventDownStub ENDP

Console_CL_KeyEventDisconnectedStub PROC
  test    ebp, ebp
  jnz     connected

  mov     ecx, edi
  mov     edx, ebx
  call    Console_Console_Key

  mov     rax, 1400EEADDh
  jmp     rax

connected:
  lea     rcx, [rbx+rbx*2]

  mov     rax, 1400EEACAh
  jmp     rax
Console_CL_KeyEventDisconnectedStub ENDP

END
