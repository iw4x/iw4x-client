.686
.model flat, C
option casemap:none

EXTERN ScriptError_AddThreadStartOpcodePos:PROC
EXTERN ScriptError_RecordThreadStart:PROC

.code

ScriptError_EmitThreadInternal_Stub PROC
  pushad

  push    dword ptr [esp+20h+8h]
  call    ScriptError_AddThreadStartOpcodePos
  add     esp, 4h

  push    dword ptr [esp+20h+4h]
  call    ScriptError_RecordThreadStart
  add     esp, 4h

  popad

  cmp     byte ptr ds:[1CFEEF0h], 2

  push    61A687h
  ret
ScriptError_EmitThreadInternal_Stub ENDP

END
