.686
.model flat, C
option casemap:none

EXTERN Security_UI_ReplaceDirective_Hook:PROC

.code

Security_G_GetClientScore_Hk PROC
  mov     eax, [esp+4]
  mov     ecx, dword ptr ds:[1A831A8h]

  test    ecx, ecx
  jz      invalid_ptr

  imul    eax, 366Ch
  mov     eax, [eax+ecx+3134h]
  ret

invalid_ptr:
  xor     eax, eax
  ret
Security_G_GetClientScore_Hk ENDP

Security_UI_ReplaceDirective_Stub PROC
  pushad
  push    dword ptr [esp+10h+20h]
  push    dword ptr [esp+10h+20h]
  push    dword ptr [esp+10h+20h]
  push    dword ptr [esp+10h+20h]

  call    Security_UI_ReplaceDirective_Hook

  add     esp, 10h
  popad

  sub     esp, 208h

  push    412C36h
  ret
Security_UI_ReplaceDirective_Stub ENDP

END
