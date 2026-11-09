.686
.model flat, C
option casemap:none

EXTERN ScriptPatches_RemoveRefToObject_Bad:PROC

.code

ScriptPatches_RemoveRefToObject_Stub PROC
  mov     eax, [esp+4h]
  cmp     eax, 9002h
  jae     bad

  push    esi
  push    edi
  mov     edi, [esp+0Ch]

  push    437196h
  ret

bad:
  push    dword ptr [esp]
  push    eax
  call    ScriptPatches_RemoveRefToObject_Bad

  add     esp, 8h
  ret
ScriptPatches_RemoveRefToObject_Stub ENDP

END
