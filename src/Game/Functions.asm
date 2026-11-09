.686
.model flat, C
option casemap:none

.code

UI_GetScoreboardLeft PROC
  push    eax
  pushad

  mov     ecx, 590390h
  mov     eax, [esp+28h]
  call    ecx
  mov     [esp+20h], eax
  popad
  pop     eax

  ret
UI_GetScoreboardLeft ENDP

PM_IsAdsAllowed PROC
  push    eax
  pushad

  mov     esi, [esp+24h+4h]
  mov     ecx, 5755A0h
  call    ecx

  mov     [esp+20h], eax
  popad
  pop     eax

  ret
PM_IsAdsAllowed ENDP

R_LoadSunThroughDvars PROC
  pushad
  push    dword ptr [esp+28h]
  mov     eax, [esp+28h]

  mov     ecx, 53F990h
  call    ecx

  add     esp, 4h
  popad
  ret
R_LoadSunThroughDvars ENDP

R_SetSunFromDvars PROC
  pushad
  mov     esi, [esp+24h]

  mov     eax, 53F6D0h
  call    eax

  popad
  ret
R_SetSunFromDvars ENDP

IN_KeyUp PROC
  pushad
  mov     esi, [esp+24h]
  mov     eax, 5A5580h
  call    eax
  popad
  ret
IN_KeyUp ENDP

IN_KeyDown PROC
  pushad
  mov     esi, [esp+24h]
  mov     eax, 5A54E0h
  call    eax
  popad
  ret
IN_KeyDown ENDP

Load_VertexBuffer PROC
  pushad

  mov     eax, [esp+2Ch]
  mov     edi, [esp+28h]
  push    dword ptr [esp+24h]

  mov     ebx, 5112C0h
  call    ebx
  add     esp, 4

  popad

  ret
Load_VertexBuffer ENDP

Image_Setup PROC
  pushad

  mov     eax, [esp+24h]
  mov     edi, [esp+28h]
  push    dword ptr [esp+38h]
  push    dword ptr [esp+38h]
  push    dword ptr [esp+38h]
  push    dword ptr [esp+38h]

  mov     ecx, 54AF50h
  call    ecx

  add     esp, 10h

  popad
  ret
Image_Setup ENDP

Menu_FreeItem PROC item:DWORD
  pushad
  mov     edi, item
  mov     ebx, 63D880h
  call    ebx
  popad
  ret
Menu_FreeItem ENDP

Menu_SetNextCursorItem PROC USES ebx a1:DWORD, a2:DWORD, unk:DWORD
  push    unk
  push    a2
  mov     eax, a1
  mov     ebx, 639FE0h
  call    ebx
  add     esp, 8h
  ret
Menu_SetNextCursorItem ENDP

Menu_SetPrevCursorItem PROC USES ebx a1:DWORD, a2:DWORD, unk:DWORD
  push    unk
  push    a2
  mov     eax, a1
  mov     ebx, 639F20h
  call    ebx
  add     esp, 8h
  ret
Menu_SetPrevCursorItem ENDP

R_AddDebugLine PROC
  pushad

  mov     eax, [esp+2Ch]
  push    eax

  mov     eax, [esp+2Ch]
  push    eax

  mov     esi, dword ptr ds:[66DAD78h]
  add     esi, 268772

  mov     edi, [esp+2Ch]

  mov     eax, 51CEB0h
  call    eax

  add     esp, 8h
  popad
  ret
R_AddDebugLine ENDP

R_AddDebugString PROC
  pushad

  mov     eax, [esp+30h]
  push    eax

  mov     eax, [esp+30h]
  push    eax

  mov     eax, [esp+2Ch]
  push    eax

  mov     eax, [esp+34h]
  push    eax

  mov     edi, dword ptr ds:[66DAD78h]
  add     edi, 268772

  mov     eax, 51D0A0h
  call    eax

  add     esp, 10h
  popad
  ret
R_AddDebugString ENDP

R_GetCharacterGlyph PROC font:DWORD, letter:DWORD
  LOCAL   result:DWORD

  pushad
  mov     edi, letter
  push    font
  call    dword ptr [gameR_GetCharacterGlyph]
  add     esp, 4
  mov     result, eax
  popad

  mov     eax, result
  ret

gameR_GetCharacterGlyph:
  dd      5055C0h
R_GetCharacterGlyph ENDP

SetupPulseFXVars PROC
  push    eax
  pushad

  mov     eax, [esp+08h+24h]
  push    dword ptr [esp+2Ch+24h]
  push    dword ptr [esp+2Ch+24h]
  push    dword ptr [esp+2Ch+24h]
  push    dword ptr [esp+2Ch+24h]
  push    dword ptr [esp+2Ch+24h]
  push    dword ptr [esp+2Ch+24h]
  push    dword ptr [esp+2Ch+24h]
  push    dword ptr [esp+2Ch+24h]
  push    dword ptr [esp+2Ch+24h]
  push    dword ptr [esp+28h+24h]

  mov     ebx, 535050h
  call    ebx
  add     esp, 28h

  mov     [esp+20h], eax
  popad
  pop     eax
  ret
SetupPulseFXVars ENDP

RB_DrawChar PROC
  pushad

  mov     eax, [esp+4h+20h]
  mov     edx, [esp+20h+20h]

  push    dword ptr [esp+24h+20h]
  push    dword ptr [esp+20h+20h]
  push    dword ptr [esp+20h+20h]
  push    dword ptr [esp+20h+20h]
  push    dword ptr [esp+20h+20h]
  push    dword ptr [esp+20h+20h]
  push    dword ptr [esp+20h+20h]

  mov     ecx, 534E20h
  call    ecx
  add     esp, 1Ch

  popad
  ret
RB_DrawChar ENDP

RB_DrawStretchPicRotate PROC
  pushad

  mov     eax, [esp+4h+20h]
  push    dword ptr [esp+30h+20h]
  push    dword ptr [esp+30h+20h]
  push    dword ptr [esp+30h+20h]
  push    dword ptr [esp+30h+20h]
  push    dword ptr [esp+30h+20h]
  push    dword ptr [esp+30h+20h]
  push    dword ptr [esp+30h+20h]
  push    dword ptr [esp+30h+20h]
  push    dword ptr [esp+30h+20h]
  push    dword ptr [esp+30h+20h]
  push    dword ptr [esp+30h+20h]

  mov     ebx, 5310F0h
  call    ebx
  add     esp, 2Ch

  popad
  ret
RB_DrawStretchPicRotate ENDP

ModulateByteColors PROC
  push    eax
  pushad

  mov     eax, [esp+4h+24h]
  mov     ecx, [esp+8h+24h]

  mov     ebx, 5353C0h
  call    ebx

  mov     [esp+20h], eax
  popad
  pop     eax

  ret
ModulateByteColors ENDP

AimAssist_UpdateTweakables PROC
  mov     eax, [esp+4h]
  call    dword ptr [gameAimAssist_UpdateTweakables]
  ret

gameAimAssist_UpdateTweakables:
  dd      569950h
AimAssist_UpdateTweakables ENDP

AimAssist_UpdateAdsLerp PROC
  mov     eax, [esp+4h]
  call    dword ptr [gameAimAssist_UpdateAdsLerp]
  ret

gameAimAssist_UpdateAdsLerp:
  dd      569AA0h
AimAssist_UpdateAdsLerp ENDP

ApplyTokenToField PROC
  push    eax
  pushad

  mov     eax, [esp+24h+4h]
  mov     ecx, [esp+24h+8h]
  push    dword ptr [esp+24h+0Ch]
  call    dword ptr [gameApplyTokenToField]
  add     esp, 4h

  movzx   eax, al

  mov     [esp+20h], eax
  popad
  pop     eax

  ret

gameApplyTokenToField:
  dd      59A760h
ApplyTokenToField ENDP

SEH_GetLocalizedTokenReference PROC token:DWORD, reference:DWORD, messageType:DWORD, errType:DWORD
  LOCAL   result:DWORD

  pushad
  mov     esi, reference
  mov     edi, messageType
  mov     ebx, errType
  push    token
  call    dword ptr [gameSEH_GetLocalizedTokenReference]
  add     esp, 4h
  mov     result, eax
  popad

  mov     eax, result
  ret

gameSEH_GetLocalizedTokenReference:
  dd      629BB0h
SEH_GetLocalizedTokenReference ENDP

Player_SwitchToWeapon PROC player:DWORD
  pushad
  mov     ebx, player
  call    dword ptr [gamePlayer_SwitchToWeapon]
  popad
  ret

gamePlayer_SwitchToWeapon:
  dd      5D97B0h
Player_SwitchToWeapon ENDP

END
