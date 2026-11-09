.686
.model flat, C
option casemap:none

EXTERN UIScript_RunMenuScript:PROC

.code

UIScript_RunMenuScriptStub PROC
  mov     eax, esp
  add     eax, 8h
  mov     edx, eax
  mov     eax, [esp+0C10h]

  push    eax
  push    edx
  call    UIScript_RunMenuScript
  add     esp, 8h

  test    al, al
  jz      continue

  pop     edi
  pop     esi
  add     esp, 0C00h
  ret

continue:
  mov     eax, 45ED00h
  jmp     eax
UIScript_RunMenuScriptStub ENDP

END
