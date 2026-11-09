.686
.model flat, C
option casemap:none

.code

CL_MouseMove PROC localClientNum:DWORD, cmd:DWORD, frametime_base:DWORD
  pushad
  mov     ebx, cmd
  mov     eax, localClientNum
  push    frametime_base
  call    dword ptr [gameCL_MouseMove]
  add     esp, 4h
  popad
  ret

gameCL_MouseMove:
  dd      5A6240h
CL_MouseMove ENDP

AdjustViewanglesForKeyboard PROC localClientNum:DWORD
  pushad
  mov     eax, localClientNum
  call    dword ptr [gameAdjustViewanglesForKeyboard]
  popad
  ret

gameAdjustViewanglesForKeyboard:
  dd      5A5D80h
AdjustViewanglesForKeyboard ENDP

END
