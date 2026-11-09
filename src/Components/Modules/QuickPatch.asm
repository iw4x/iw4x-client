.686
.model flat, C
option casemap:none

EXTERN QuickPatch_g_antilag:DWORD

EXTERN QuickPatch_SetAspectRatio:PROC
EXTERN QuickPatch_CL_KeyEvent_OnEscape:PROC

ERR_DROP equ 1

.const

aliasNotFound db "SND_GetAliasOffset: Could not find sound alias '%s'", 0

.code

QuickPatch_ClientEventsFireWeapon_Stub PROC
  mov     eax, QuickPatch_g_antilag
  cmp     byte ptr [eax+16], 1

  je      fireWeapon

  mov     eax, 1A83554h
  mov     ecx, [eax]

fireWeapon:
  push    edx
  push    ecx
  push    edi
  mov     eax, 4A4D50h
  call    eax
  add     esp, 0Ch
  pop     edi
  pop     ecx
  ret
QuickPatch_ClientEventsFireWeapon_Stub ENDP

QuickPatch_ClientEventsFireWeaponMelee_Stub PROC
  mov     eax, QuickPatch_g_antilag
  cmp     byte ptr [eax+16], 1

  je      fireWeaponMelee

  mov     eax, 1A83554h
  mov     edx, [eax]

fireWeaponMelee:
  push    edx
  push    edi
  mov     eax, 4F2470h
  call    eax
  add     esp, 8
  pop     edi
  pop     ecx
  ret
QuickPatch_ClientEventsFireWeaponMelee_Stub ENDP

QuickPatch_SetAspectRatio_Stub PROC
  cmp     eax, 4
  ja      goToDefaultCase
  je      useCustomRatio

  push    5063FCh
  ret

goToDefaultCase:
  push    5064FCh
  ret

useCustomRatio:
  pushad
  call    QuickPatch_SetAspectRatio
  popad

  mov     eax, 1

  push    506495h
  ret
QuickPatch_SetAspectRatio_Stub ENDP

QuickPatch_CL_KeyEvent_ConsoleEscape_Stub PROC
  pushad
  call    QuickPatch_CL_KeyEvent_OnEscape
  popad

  mov     ebx, 4F66F2h
  jmp     ebx
QuickPatch_CL_KeyEvent_ConsoleEscape_Stub ENDP

QuickPatch_SND_GetAliasOffset_Stub PROC
  test    eax, eax
  jz      error

  mov     ecx, eax
  mov     edx, dword ptr [ecx+4h]

  push    437CB2h
  ret

error:
  add     esp, 4h

  push    dword ptr [esi]
  push    OFFSET aliasNotFound
  push    ERR_DROP
  call    dword ptr [gameCom_Error]
  add     esp, 0Ch

  xor     eax, eax
  pop     esi
  ret

gameCom_Error:
  dd      4B22D0h
QuickPatch_SND_GetAliasOffset_Stub ENDP

QuickPatch_VehicleFx_PlayerIndexCheck_Stub PROC
  mov     ecx, [esi+38h]
  cmp     ecx, 18
  jb      validIndex

  test    eax, eax
  jmp     done

validIndex:
  imul    ecx, ecx, 52Ch
  cmp     eax, [ecx+8E77CCh]

done:
  push    428555h
  ret
QuickPatch_VehicleFx_PlayerIndexCheck_Stub ENDP

QuickPatch_VehicleCl_SetPlayerIndex_UpdateEntity_Stub PROC
  cmp     eax, 18
  jb      updateValid
  xor     eax, eax

updateValid:
  mov     [ebx+38h], eax
  lea     edi, [ebx+1Ch]

  push    679EACh
  ret
QuickPatch_VehicleCl_SetPlayerIndex_UpdateEntity_Stub ENDP

QuickPatch_VehicleCl_SetPlayerIndex_ResetEntity_Stub PROC
  cmp     ecx, 18
  jb      resetValid
  xor     ecx, ecx

resetValid:
  mov     [esi+38h], ecx

  mov     eax, 402500h
  call    eax

  push    679E34h
  ret
QuickPatch_VehicleCl_SetPlayerIndex_ResetEntity_Stub ENDP

END
