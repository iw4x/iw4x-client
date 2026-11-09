.686
.model flat, C
option casemap:none

EXTERN SlowMotion_Com_UpdateSlowMotion:PROC

.code

SlowMotion_Com_UpdateSlowMotion_Stub PROC
  pushad

  push    dword ptr [esp+20h+4h]
  call    SlowMotion_Com_UpdateSlowMotion
  add     esp, 4h

  popad

  ret
SlowMotion_Com_UpdateSlowMotion_Stub ENDP

END
