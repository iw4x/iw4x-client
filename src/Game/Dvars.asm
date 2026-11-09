.686
.model flat, C
option casemap:none

.code

Dvar_SetVariant PROC
  pushad

  mov     eax, [esp+4h+20h]
  push    dword ptr [esp+18h+20h]
  push    dword ptr [esp+18h+20h]
  push    dword ptr [esp+18h+20h]
  push    dword ptr [esp+18h+20h]
  push    dword ptr [esp+18h+20h]

  call    dword ptr [gameDvar_SetVariant]
  add     esp, 14h

  popad

  ret

gameDvar_SetVariant:
  dd      647400h
Dvar_SetVariant ENDP

Dvar_SetFromStringFromSource PROC dvar:DWORD, string:DWORD, source:DWORD
  pushad
  mov     esi, dvar
  mov     eax, string
  push    source
  call    dword ptr [gameDvar_SetFromStringFromSource]
  add     esp, 4h
  popad
  ret

gameDvar_SetFromStringFromSource:
  dd      648580h
Dvar_SetFromStringFromSource ENDP

END
