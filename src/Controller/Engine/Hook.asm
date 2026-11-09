.686
.model flat, C
option casemap:none

EXTERN Controller_ShouldUse:PROC
EXTERN Controller_CommandAssignment:PROC
EXTERN Controller_MenuCursorFocusPoint:PROC
EXTERN Controller_MouseMove:PROC
EXTERN Controller_RemoteControlMove:PROC
EXTERN Controller_LocationSelection:PROC
EXTERN Controller_ReadDeltaMovement:PROC
EXTERN Controller_LocalizedKeyNames:PROC

.code

Controller_PlayerUseEntityStub PROC
  cmp     eax, [ecx+10h]
  jl      skip

  push    eax
  pushad

  push    eax
  push    edi
  call    Controller_ShouldUse
  add     esp, 8h

  mov     [esp+20h], eax

  popad
  pop     eax

  test    al, al
  jz      skip

  push    5FE39Bh
  ret

skip:
  push    5FE3AFh
  ret
Controller_PlayerUseEntityStub ENDP

Controller_CommandAssignmentStub PROC
  push    eax
  pushad

  push    dword ptr [esp+20h+4h+8h]
  push    dword ptr [esp+20h+4h+8h]
  push    eax
  call    Controller_CommandAssignment
  add     esp, 0Ch

  mov     [esp+20h], eax

  popad
  pop     eax
  ret
Controller_CommandAssignmentStub ENDP

Controller_ItemSetFocusStub PROC
  pushad

  lea     eax, [esp+20h+4h]
  push    eax
  push    ebx
  call    Controller_MenuCursorFocusPoint
  add     esp, 8h

  popad

  push    6392B0h
  ret
Controller_ItemSetFocusStub ENDP

Controller_MouseMoveStub PROC
  pushad

  push    dword ptr [esp+20h+4h]
  push    ebx
  push    eax
  call    Controller_MouseMove
  add     esp, 0Ch

  popad
  ret
Controller_MouseMoveStub ENDP

Controller_RemoteControlMoveStub PROC
  push    edi
  push    eax

  call    dword ptr [gameCL_RemoteControlMove]
  call    Controller_RemoteControlMove

  add     esp, 8h
  ret

gameCL_RemoteControlMove:
  dd      5A6BA0h
Controller_RemoteControlMoveStub ENDP

Controller_LocationSelectionStub PROC
  push    esi
  push    eax

  call    dword ptr [gameCL_LocationSelection]

  test    al, al
  jz      done

  call    Controller_LocationSelection

done:
  add     esp, 8h
  ret

gameCL_LocationSelection:
  dd      5A67A0h
Controller_LocationSelectionStub ENDP

Controller_WriteDeltaMovementStub PROC
  add     esp, 0Ch

  mov     dl, byte ptr [edi+1Ah]
  mov     dh, byte ptr [edi+1Bh]
  mov     [esp+30h], dx

  mov     dl, byte ptr [ebp+1Ah]
  mov     dh, byte ptr [ebp+1Bh]
  mov     [esp+2Ch], dx

  push    60E40Eh
  ret
Controller_WriteDeltaMovementStub ENDP

Controller_ReadDeltaMovementStub1 PROC
  push    ebx
  push    ebp
  push    edi
  push    esi
  call    Controller_ReadDeltaMovement
  add     esp, 10h

  push    4921BFh
  ret
Controller_ReadDeltaMovementStub1 ENDP

Controller_ReadDeltaMovementStub2 PROC
  push    ebx
  push    ebp
  push    edi
  push    esi
  call    Controller_ReadDeltaMovement
  add     esp, 10h

  push    3
  push    esi
  push    492085h
  ret
Controller_ReadDeltaMovementStub2 ENDP

Controller_LocalizedKeyNamesStub PROC
  push    eax
  pushad

  call    Controller_LocalizedKeyNames
  mov     [esp+20h], eax

  popad
  pop     eax

  test    edi, edi
  ret
Controller_LocalizedKeyNamesStub ENDP

END
