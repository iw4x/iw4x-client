.686
.model flat, C
option casemap:none

EXTERN Dvar_OnRegisterVariant:PROC

.code

Dvar_RegisterVariant_Stub PROC
  pushad

  push    eax
  call    Dvar_OnRegisterVariant
  add     esp, 4h

  popad

  pop     edi
  pop     esi
  pop     ebp
  pop     ebx
  ret
Dvar_RegisterVariant_Stub ENDP

END
