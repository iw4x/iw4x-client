.686
.model flat, C
option casemap:none

.code

FS_AddLocalizedGameDirectory PROC path:DWORD, dir:DWORD
  pushad
  mov     ebx, path
  mov     eax, dir
  call    dword ptr [gameFS_AddLocalizedGameDirectory]
  popad
  ret

gameFS_AddLocalizedGameDirectory:
  dd      642EF0h
FS_AddLocalizedGameDirectory ENDP

END
