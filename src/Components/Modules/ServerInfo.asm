.686
.model flat, C
option casemap:none

EXTERN ServerInfo_DrawScoreboardInfo:PROC

.code

ServerInfo_DrawScoreboardStub PROC
  pushad
  push    eax
  call    ServerInfo_DrawScoreboardInfo
  pop     eax
  popad

  push    591B70h
  ret
ServerInfo_DrawScoreboardStub ENDP

END
