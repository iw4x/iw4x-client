option casemap:none

EXTERN Maps_RotateConnectionState:DWORD

EXTERN Maps_GetWorldData:PROC
EXTERN Maps_TriggerReconnectForMap:PROC

.code

Maps_GetWorldDataStub PROC
  sub     rsp, 28h
  call    Maps_GetWorldData
  add     rsp, 28h

  mov     rcx, rax
  ret
Maps_GetWorldDataStub ENDP

Maps_SetConnectedStub PROC
  mov     eax, dword ptr [rcx+rbx+6CCB78h]
  mov     Maps_RotateConnectionState, eax

  mov     dword ptr [rcx+rbx+6CCB78h], 5
  ret
Maps_SetConnectedStub ENDP

Maps_RequestStatsStub PROC
  mov     rax, 1406CCB78h
  mov     eax, dword ptr [rax]
  cmp     eax, 6
  je      skip
  cmp     eax, 5
  jl      skip

  mov     rax, 1400FA0A4h
  jmp     rax

skip:
  mov     rax, 1400FA0FBh
  jmp     rax
Maps_RequestStatsStub ENDP

Maps_RotateCheckStub PROC
  push    rax
  sub     rsp, 28h

  mov     rcx, r12
  lea     rdx, [rbp-30h]
  call    Maps_TriggerReconnectForMap

  add     rsp, 28h
  mov     ecx, eax
  pop     rax

  test    ecx, ecx
  jnz     skipRotation

  xor     ecx, ecx
  mov     rdx, 1406CC028h
  mov     byte ptr [rdx], 1

  mov     rdx, 1400F9FB8h
  jmp     rdx

skipRotation:
  mov     rdx, 1400F96F3h
  jmp     rdx
Maps_RotateCheckStub ENDP

END
