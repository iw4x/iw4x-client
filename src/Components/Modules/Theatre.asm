.686
.model flat, C
option casemap:none

EXTERN Theatre_StoreBaseline:PROC
EXTERN Theatre_WriteBaseline:PROC
EXTERN Theatre_AdjustTimeDelta:PROC

.code

Theatre_BaselineStoreStub PROC
  push    edi
  call    Theatre_StoreBaseline
  pop     edi

  mov     edx, 5ABEF5h
  jmp     edx
Theatre_BaselineStoreStub ENDP

Theatre_BaselineToFileStub PROC
  pushad
  call    Theatre_WriteBaseline
  popad

  mov     ecx, 0A5E9C4h
  mov     byte ptr [ecx], 0

  push    5A863Ah
  ret
Theatre_BaselineToFileStub ENDP

Theatre_AdjustTimeDeltaStub PROC
  push    eax
  pushad
  call    Theatre_AdjustTimeDelta
  mov     [esp+20h], eax
  popad
  pop     eax

  test    al, al
  jz      continue

  ret

continue:
  push    5A1AD0h
  ret
Theatre_AdjustTimeDeltaStub ENDP

Theatre_ServerTimedOutStub PROC
  mov     eax, 0A5EA0Ch
  mov     eax, [eax]
  test    al, al
  jz      continue

  mov     eax, 5A8E70h
  jmp     eax

continue:
  mov     eax, dword ptr ds:[0B2BB90h]
  push    5A8E08h
  ret
Theatre_ServerTimedOutStub ENDP

Theatre_UISetActiveMenuStub PROC
  mov     eax, 0A5EA0Ch
  mov     eax, [eax]
  test    al, al
  jz      continue

  mov     eax, 4CB49Ch
  jmp     eax

continue:
  mov     ecx, [esp+10h]
  push    10h
  push    ecx
  push    4CB3F6h
  ret
Theatre_UISetActiveMenuStub ENDP

END
