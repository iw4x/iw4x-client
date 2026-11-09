.686
.model flat, C
option casemap:none

EXTERN Window_IsNoBorder:PROC

WS_VISIBLE equ 10000000h
WS_POPUP equ 80000000h
WS_SYSMENU equ 80000h
WS_CAPTION equ 0C00000h
WS_MINIMIZEBOX equ 20000h

.code

Window_StyleHookStub PROC
  call    Window_IsNoBorder
  test    al, al
  jz      setBorder

  mov     ebp, WS_VISIBLE OR WS_POPUP
  ret

setBorder:
  mov     ebp, WS_VISIBLE OR WS_SYSMENU OR WS_CAPTION OR WS_MINIMIZEBOX
  ret
Window_StyleHookStub ENDP

END
