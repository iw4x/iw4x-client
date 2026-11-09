.686
.model flat, C
option casemap:none

EXTERN FastFiles_AuthLoadInflateDecryptBaseFunc:PROC

.code

FastFiles_AuthLoadInflateDecryptBase PROC
  pushad
  push    ebx
  call    FastFiles_AuthLoadInflateDecryptBaseFunc
  pop     ebx
  popad

  push    5B96F0h
  ret
FastFiles_AuthLoadInflateDecryptBase ENDP

FastFiles_ReadXFile PROC
  mov     eax, [esp+4]
  mov     ecx, [esp+8]

  push    445468h
  ret
FastFiles_ReadXFile ENDP

END
