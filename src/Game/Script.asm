.686
.model flat, C
option casemap:none

.code

RuntimeErrorInternal PROC channel:DWORD, codePos:DWORD, index:DWORD, msg:DWORD
  pushad
  mov     eax, msg
  mov     edi, channel
  push    index
  push    codePos
  call    dword ptr [gameRuntimeErrorInternal]
  add     esp, 8h
  popad
  ret

gameRuntimeErrorInternal:
  dd      61ABE0h
RuntimeErrorInternal ENDP

Scr_NotifyId PROC
  pushad

  mov     eax, [esp+20h+0Ch]
  push    dword ptr [esp+20h+8h]
  push    dword ptr [esp+20h+8h]
  call    dword ptr [gameScr_NotifyId]
  add     esp, 8h

  popad
  ret

gameScr_NotifyId:
  dd      61E670h
Scr_NotifyId ENDP

END
