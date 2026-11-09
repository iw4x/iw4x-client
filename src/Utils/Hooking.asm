.686
.model flat, C
option casemap:none

EXTERN Hooking_RunCallback:PROC
EXTERN Hooking_PopReturn:PROC

.code

Hooking_InterceptionStub PROC
  sub     esp, 4h
  pushad

  lea     eax, [esp+20h]
  push    eax

  call    Hooking_RunCallback
  call    Hooking_PopReturn

  add     esp, 4h

  mov     [esp+20h], eax
  popad

  ret
Hooking_InterceptionStub ENDP

END
