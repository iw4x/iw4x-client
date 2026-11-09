.686
.model flat, C
option casemap:none

EXTERN CardTitles_GetPlayerCardClientInfo:PROC
EXTERN CardTitles_TableLookupByRowHook:PROC

.code

CardTitles_GetPlayerCardClientInfoStub PROC
  push    eax
  pushad

  push    esi
  push    eax
  call    CardTitles_GetPlayerCardClientInfo
  add     esp, 8

  mov     [esp+20h], eax
  popad
  pop     eax

  pop     esi
  pop     ebp
  mov     [ebx+4], eax
  pop     ebx

  ret
CardTitles_GetPlayerCardClientInfoStub ENDP

CardTitles_TableLookupByRowHookStub PROC
  push    eax
  pushad

  push    esi
  push    ebx

  call    CardTitles_TableLookupByRowHook
  add     esp, 8

  mov     [esp+20h], eax
  popad
  pop     eax

  cmp     eax, 0
  jz      originalTitle

  pop     ecx
  mov     ecx, dword ptr [esi+4]
  ret

originalTitle:
  mov     eax, [esi+50h]
  cmp     eax, 3

  push    62DCC7h
  ret
CardTitles_TableLookupByRowHookStub ENDP

END
