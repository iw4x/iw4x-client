.686
.model flat, C
option casemap:none

EXTERN ConfigStrings_CG_ParseExtraConfigStrings:PROC

.code

ConfigStrings_CG_ParseConfigStrings PROC
  push    eax
  pushad

  call    ConfigStrings_CG_ParseExtraConfigStrings

  mov     [esp+20h], eax
  popad

  pop     eax

  test    eax, eax
  jz      continueParsing

  ret

continueParsing:
  push    592960h
  ret
ConfigStrings_CG_ParseConfigStrings ENDP

END
