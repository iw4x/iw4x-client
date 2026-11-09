.686
.model flat, C
option casemap:none

EXTERN Elevators_PM_CorrectAllSolid:PROC

.code

Elevators_PM_CorrectAllSolidStub PROC
  push    eax
  pushad

  push    dword ptr [esp+8h+24h]
  push    dword ptr [esp+8h+24h]
  push    eax
  call    Elevators_PM_CorrectAllSolid
  add     esp, 0Ch

  mov     [esp+20h], eax
  popad
  pop     eax

  ret
Elevators_PM_CorrectAllSolidStub ENDP

END
