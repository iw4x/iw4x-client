option casemap:none

EXTERN Script_Scr_GetMethod_Hk:PROC

.code

Script_Scr_GetMethod_Stub PROC
  mov     rdx, r14
  jmp     Script_Scr_GetMethod_Hk
Script_Scr_GetMethod_Stub ENDP

END
