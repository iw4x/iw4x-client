EXTERN QuickPatch_SetAspectRatio:PROC
EXTERN QuickPatch_CL_KeyEvent_OnEscape:PROC

.code

QuickPatch_SetAspectRatio_Stub PROC
  cmp     edx, 1
  je      wide

  cmp     edx, 2
  je      custom

  mov     rax, 140031E96h
  jmp     rax

wide:
  mov     rax, 140031F1Ch
  jmp     rax

custom:
  sub     rsp, 20h
  call    QuickPatch_SetAspectRatio
  add     rsp, 20h

  mov     dl, 1

  mov     rax, 140031F42h
  jmp     rax
QuickPatch_SetAspectRatio_Stub ENDP

QuickPatch_CL_KeyEvent_ConsoleEscape_Stub PROC
  test    dl, 1
  jz      no_console

  sub     rsp, 20h
  call    QuickPatch_CL_KeyEvent_OnEscape
  add     rsp, 20h

  mov     rax, 1400EEADDh
  jmp     rax

no_console:
  test    dl, 10h
  jnz     ui

  mov     rax, 1400EE962h
  jmp     rax

ui:
  mov     rax, 1400EE9C4h
  jmp     rax
QuickPatch_CL_KeyEvent_ConsoleEscape_Stub ENDP

QuickPatch_VehicleFx_PlayerIndexCheck_Stub PROC
  cmp     eax, 18
  jb      valid

  test    r9d, r9d
  ret

valid:
  imul    rdx, rax, 548h
  cmp     r9d, [rdx+r13+109B4Ch]
  ret
QuickPatch_VehicleFx_PlayerIndexCheck_Stub ENDP

QuickPatch_VehicleCl_SetPlayerIndex_UpdateEntity_Stub PROC
  mov     eax, [rdx+158h]
  cmp     eax, 18
  jb      valid
  xor     eax, eax

valid:
  mov     [rbx+r12+6742308h], eax
  ret
QuickPatch_VehicleCl_SetPlayerIndex_UpdateEntity_Stub ENDP

QuickPatch_VehicleCl_SetPlayerIndex_ResetEntity_Stub PROC
  mov     eax, [rbp+158h]
  cmp     eax, 18
  jb      valid
  xor     eax, eax

valid:
  mov     [rsi+38h], eax
  ret
QuickPatch_VehicleCl_SetPlayerIndex_ResetEntity_Stub ENDP

END
