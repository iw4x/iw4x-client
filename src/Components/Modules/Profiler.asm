.686
.model flat, C
option casemap:none

.code

Profiler_WaitStub PROC
  push    esi

  mov     esi, dword ptr [esp+8h]
  push    dword ptr [esp+0Ch]
  mov     eax, 52CA10h
  call    eax
  add     esp, 4h

  pop     esi
  ret
Profiler_WaitStub ENDP

END
