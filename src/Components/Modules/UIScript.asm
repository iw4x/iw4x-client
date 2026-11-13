option casemap:none

EXTERN UIScript_RunMenuScript:PROC

.code

UIScript_RunMenuScriptStub PROC
  lea     rcx, [rbp+30h]
  mov     rdx, rbx
  sub     rsp, 20h
  call    UIScript_RunMenuScript
  add     rsp, 20h

  test    al, al
  jz      continue

  mov     rax, 140270565h
  jmp     rax

continue:
  mov     [rsp+0D60h], rsi
  mov     rax, 1402703EFh
  jmp     rax
UIScript_RunMenuScriptStub ENDP

END
