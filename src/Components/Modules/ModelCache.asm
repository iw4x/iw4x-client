.686
.model flat, C
option casemap:none

EXTERN ModelCache_R_RegisterModel_InitGraphics:PROC

.code

ModelCache_R_RegisterModel_Hook PROC
  pushad
  push    esi
  push    eax
  call    ModelCache_R_RegisterModel_InitGraphics
  pop     esi
  pop     eax
  popad

  push    58991Bh
  ret
ModelCache_R_RegisterModel_Hook ENDP

END
