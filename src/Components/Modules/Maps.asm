.686
.model flat, C
option casemap:none

EXTERN Maps_RotateConnectionState:DWORD

EXTERN Maps_GetWorldData:PROC
EXTERN Maps_TriggerReconnectForMap:PROC
EXTERN Maps_PrepareUsermap:PROC
EXTERN Maps_HideModel:PROC

.code

Maps_GetWorldDataStub PROC
  push    eax
  pushad

  call    Maps_GetWorldData

  mov     [esp+20h], eax
  popad
  pop     eax

  ret
Maps_GetWorldDataStub ENDP

Maps_SetConnectedStub PROC
  push    eax
  mov     eax, dword ptr [esi+0B2C540h]
  mov     Maps_RotateConnectionState, eax
  pop     eax

  mov     dword ptr [esi+0B2C540h], 5
  ret
Maps_SetConnectedStub ENDP

Maps_RequestStatsStub PROC
  mov     eax, dword ptr ds:[0B2C540h]
  cmp     eax, 6
  je      skip
  cmp     eax, 5
  jl      skip

  push    5AA94Ch
  ret

skip:
  push    5AA9A9h
  ret
Maps_RequestStatsStub ENDP

Maps_RotateCheckStub PROC
  push    eax
  pushad

  push    dword ptr [esp+28h]
  push    ebp
  call    Maps_TriggerReconnectForMap
  add     esp, 8h

  mov     [esp+20h], eax

  popad
  pop     eax

  test    eax, eax
  jnz     skipRotation

  push    487C50h

skipRotation:
  ret
Maps_RotateCheckStub ENDP

Maps_SpawnServerStub PROC
  pushad

  push    dword ptr [esp+24h]
  call    Maps_PrepareUsermap
  pop     eax

  popad

  push    4A7120h
  ret
Maps_SpawnServerStub ENDP

Maps_LoadMapLoadscreenStub PROC
  pushad

  push    dword ptr [esp+24h]
  call    Maps_PrepareUsermap
  pop     eax

  popad

  push    4D8030h
  ret
Maps_LoadMapLoadscreenStub ENDP

Maps_HideModelStub PROC
  pushad
  call    Maps_HideModel
  popad

  push    541E40h
  ret
Maps_HideModelStub ENDP

END
