.686
.model flat, C
option casemap:none

EXTERN Console_MakeRoomForText:PROC
EXTERN Console_StoreSafeArea:PROC
EXTERN Console_RestoreSafeArea:PROC

.code

Console_Sys_PrintStub PROC
  pushad
  push    edi
  call    Console_MakeRoomForText
  pop     edi
  popad

  push    4F57F8h
  ret
Console_Sys_PrintStub ENDP

Console_DrawSolidConsoleStub PROC
  pushad
  call    Console_StoreSafeArea
  popad

  call    dword ptr [gameCon_DrawSolidConsole]

  pushad
  call    Console_RestoreSafeArea
  popad
  ret

gameCon_DrawSolidConsole:
  dd      5A5040h
Console_DrawSolidConsoleStub ENDP

END
