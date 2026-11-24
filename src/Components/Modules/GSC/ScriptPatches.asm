option casemap:none

EXTERN ScriptPatches_RemoveRefToObject_Bad:PROC

.code

ScriptPatches_RemoveRefToObject_Stub PROC
  cmp     ecx, 9002h
  jae     bad

  mov     [rsp+8h], rbx

  mov     rax, 1402233A5h
  jmp     rax

bad:
  mov     rdx, [rsp]
  sub     rsp, 28h
  call    ScriptPatches_RemoveRefToObject_Bad
  add     rsp, 28h
  ret
ScriptPatches_RemoveRefToObject_Stub ENDP

END
