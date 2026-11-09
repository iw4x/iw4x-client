.686
.model flat, C
option casemap:none

EXTERN Dedicated_PostInitialization:PROC
EXTERN Dedicated_Com_ClampMsec:PROC

.code

Dedicated_PostInitializationStub PROC
  pushad
  call    Dedicated_PostInitialization
  popad

  mov     eax, 43D140h
  jmp     eax
Dedicated_PostInitializationStub ENDP

Dedicated_Com_ClampMsec_Stub PROC
  pushad

  push    ecx
  call    Dedicated_Com_ClampMsec
  add     esp, 4h

  popad

  mov     edx, dword ptr ds:[1AD7934h]

  push    47DDB8h
  ret
Dedicated_Com_ClampMsec_Stub ENDP

END
