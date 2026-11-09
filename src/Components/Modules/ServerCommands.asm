.686
.model flat, C
option casemap:none

EXTERN ServerCommands_OnServerCommand:PROC

.code

ServerCommands_CG_DeployServerCommand_Stub PROC
  push    eax
  pushad

  call    ServerCommands_OnServerCommand

  mov     [esp+20h], eax
  popad
  pop     eax

  test    al, al
  jnz     jumpback

  test    eax, eax
  jle     error

  mov     eax, dword ptr [edx*4+1AAC634h]
  mov     eax, [eax]

  push    5944B3h
  ret

error:
  push    5944AEh
  ret

jumpback:
  push    594536h
  ret
ServerCommands_CG_DeployServerCommand_Stub ENDP

END
