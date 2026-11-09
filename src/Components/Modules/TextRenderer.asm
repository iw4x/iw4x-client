.686
.model flat, C
option casemap:none

EXTERN TextRenderer_ChatHandleKeyDown:PROC
EXTERN TextRenderer_SEH_PrintStrlenWithCursor:PROC
EXTERN TextRenderer_Dvar_GetUnpackedColorByName:PROC

.code

TextRenderer_Message_Key_Stub PROC
  pushad

  push    eax
  push    edi
  call    TextRenderer_ChatHandleKeyDown
  add     esp, 8h
  test    al, al
  jnz     skipHandling

  popad
  call    dword ptr [gameMessage_Key]
  ret

skipHandling:
  popad
  mov     al, 1
  ret

gameMessage_Key:
  dd      5A7E50h
TextRenderer_Message_Key_Stub ENDP

TextRenderer_Field_AdjustScroll_PrintLen_Stub PROC
  push    eax
  pushad

  push    esi
  push    dword ptr [esp+8h+24h]
  call    TextRenderer_SEH_PrintStrlenWithCursor
  add     esp, 8h
  mov     [esp+20h], eax

  popad
  pop     eax
  ret
TextRenderer_Field_AdjustScroll_PrintLen_Stub ENDP

TextRenderer_GetUnpackedColorByNameStub PROC
  push    dword ptr [esp+8h]
  push    dword ptr [esp+8h]
  call    TextRenderer_Dvar_GetUnpackedColorByName
  add     esp, 8h

  test    al, al
  jnz     continue

  ret

continue:
  push    edi
  mov     edi, [esp+8h]
  push    406535h
  ret
TextRenderer_GetUnpackedColorByNameStub ENDP

END
