.686
.model flat, C
option casemap:none

.code

ZoneBuilder_SoftErrorAssetOverflow PROC
  add     esp, 12
  pop     edi
  pop     esi
  pop     ebp
  pop     ebx
  add     esp, 14h
  mov     eax, [esp+8]
  ret
ZoneBuilder_SoftErrorAssetOverflow ENDP

END
