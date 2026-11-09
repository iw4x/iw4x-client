.686
.model flat, C
option casemap:none

EXTERN Events_CL_CmdButtons:PROC
EXTERN Events_CL_Disconnect_Hk:PROC
EXTERN Events_CL_KeyMove:PROC
EXTERN Events_NetworkStart:PROC

.code

Events_CL_CmdButtons_Stub PROC
  call    dword ptr [gameCL_CmdButtons_t]

  pushad

  push    esi
  call    Events_CL_CmdButtons
  add     esp, 4h

  popad
  ret

gameCL_CmdButtons_t:
  dd      5A6510h
Events_CL_CmdButtons_Stub ENDP

Events_CL_Disconnect_Stub PROC
  je      done

  push    dword ptr [esp+4]
  call    dword ptr [gameoriginal]
  add     esp, 4

  pushad
  push    ebx
  call    Events_CL_Disconnect_Hk
  add     esp, 4h
  popad

done:
  ret

gameoriginal:
  dd      40354Dh
Events_CL_Disconnect_Stub ENDP

Events_CL_KeyMove_Stub PROC
  call    dword ptr [gameCL_KeyMove_t]

  pushad

  push    esi
  call    Events_CL_KeyMove
  add     esp, 4h

  popad
  ret

gameCL_KeyMove_t:
  dd      5A5F40h
Events_CL_KeyMove_Stub ENDP

Events_NET_OpenSocks_Hk PROC
  mov     eax, 64D900h
  call    eax
  jmp     Events_NetworkStart
Events_NET_OpenSocks_Hk ENDP

END
