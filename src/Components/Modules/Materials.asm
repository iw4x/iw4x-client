.686
.model flat, C
option casemap:none

EXTERN Materials_WriteDeathMessageIcon:PROC

.code

Materials_ImageVersionCheck PROC
  cmp     eax, 9
  je      returnSafely

  jmp     dword ptr [gameCom_PrintError]

returnSafely:
  mov     al, 1
  add     esp, 18h
  ret

gameCom_PrintError:
  dd      4F8C70h
Materials_ImageVersionCheck ENDP

Materials_DeathMessageStub PROC
  push    eax
  pushad

  push    edx
  push    eax
  push    ecx

  call    Materials_WriteDeathMessageIcon
  add     esp, 0Ch

  mov     [esp+20h], eax
  popad
  pop     eax

  add     esp, 8h
  ret
Materials_DeathMessageStub ENDP

END
