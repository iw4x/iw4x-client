option casemap:none

EXTERN Operator_SV_GetChallenge:PROC
EXTERN Operator_ArchiveTempBans:PROC
EXTERN Operator_RestoreTempBans:PROC

.code

Operator_SV_GetChallengeStub PROC
  sub     rsp, 20h
  mov     rcx, r13
  mov     rdx, rdi
  call    Operator_SV_GetChallenge
  add     rsp, 20h
  test    al, al
  jnz     banned

  mov     rax, 1464EA2BCh
  mov     r11, 140236C15h
  jmp     r11

banned:
  mov     r11, 140236C67h
  jmp     r11
Operator_SV_GetChallengeStub ENDP

Operator_ArchiveTempBansStub PROC
  sub     rsp, 20h
  mov     rcx, rbx
  call    Operator_ArchiveTempBans
  add     rsp, 20h

  mov     r8, 1464FA870h
  mov     r11, 1401A2928h
  jmp     r11
Operator_ArchiveTempBansStub ENDP

Operator_RestoreTempBansStub PROC
  push    rax
  sub     rsp, 28h
  mov     rcx, rdi
  call    Operator_RestoreTempBans
  add     rsp, 28h
  pop     rax

  mov     r8, 1464FA870h
  mov     r11, 1401A1B3Eh
  jmp     r11
Operator_RestoreTempBansStub ENDP

END
