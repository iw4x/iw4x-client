.686
.model flat, C
option casemap:none

EXTERN ScriptExtension_ReplacedPos:DWORD

EXTERN ScriptExtension_GetReplacedPos:PROC

.code

ScriptExtension_VMExecuteInternalStub PROC
  pushad

  push    edx
  call    ScriptExtension_GetReplacedPos

  pop     edx
  popad

  cmp     ScriptExtension_ReplacedPos, 0
  jne     SetPos

  movzx   eax, byte ptr [edx]
  inc     edx

Loc1:
  cmp     eax, 8Bh

  push    ecx

  mov     ecx, 2045094h
  mov     [ecx], eax

  mov     ecx, 2040CD4h
  mov     [ecx], edx

  pop     ecx

  push    61E944h
  ret

SetPos:
  mov     edx, ScriptExtension_ReplacedPos
  mov     ScriptExtension_ReplacedPos, 0

  movzx   eax, byte ptr [edx]
  inc     edx

  jmp     Loc1
ScriptExtension_VMExecuteInternalStub ENDP

END
