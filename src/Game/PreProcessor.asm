.686
.model flat, C
option casemap:none

.code

PC_ReadLine PROC
  push    eax
  pushad

  mov     ebx, [esp+24h+4h]

  push    dword ptr [esp+24h+0Ch]
  push    dword ptr [esp+24h+0Ch]
  call    dword ptr [gamePC_ReadLine]
  add     esp, 8h

  mov     [esp+20h], eax
  popad
  pop     eax

  ret

gamePC_ReadLine:
  dd      57D830h
PC_ReadLine ENDP

PC_PushIndent PROC source:DWORD, type_:DWORD, skip:DWORD
  pushad
  mov     edi, skip
  mov     esi, source
  push    type_
  call    dword ptr [gamePC_PushIndent]
  add     esp, 4h
  popad
  ret

gamePC_PushIndent:
  dd      57D740h
PC_PushIndent ENDP

PC_PopIndent PROC source:DWORD, type_:DWORD, skip:DWORD
  pushad
  mov     edx, skip
  mov     eax, type_
  mov     ecx, source
  call    dword ptr [gamePC_PopIndent]
  popad
  ret

gamePC_PopIndent:
  dd      57D780h
PC_PopIndent ENDP

END
