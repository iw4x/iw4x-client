.686
.model flat, C
option casemap:none

.code

Com_LoadInfoString_FastFile PROC fileName:DWORD, fileDesc:DWORD, ident:DWORD, loadBuffer:DWORD
  LOCAL   result:DWORD

  pushad

  mov     edi, fileName
  mov     ebx, loadBuffer
  push    ident
  push    fileDesc
  call    dword ptr [gameCom_LoadInfoString_FastFile]
  add     esp, 8h
  mov     result, eax

  popad

  mov     eax, result
  ret

gameCom_LoadInfoString_FastFile:
  dd      609B60h
Com_LoadInfoString_FastFile ENDP

END
