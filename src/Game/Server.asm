.686
.model flat, C
option casemap:none

.code

SV_BotUserMove PROC client:DWORD
  pushad
  mov     edi, client
  call    dword ptr [gameSV_BotUserMove]
  popad
  ret

gameSV_BotUserMove:
  dd      626E50h
SV_BotUserMove ENDP

END
