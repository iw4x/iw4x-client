.686
.model flat, C
option casemap:none

EXTERN ClientCommand_GiveAll_Hk:PROC

.code

ClientCommand_GiveAll_Stub PROC
  pushad
  push    esp
  call    ClientCommand_GiveAll_Hk
  add     esp, 4
  popad
  ret
ClientCommand_GiveAll_Stub ENDP

END
