.686
.model flat, C
option casemap:none

EXTERN UIFeeder_Current:DWORD

EXTERN UIFeeder_SetItemSelection:PROC
EXTERN UIFeeder_GetItemText:PROC
EXTERN UIFeeder_GetItemCount:PROC
EXTERN UIFeeder_CheckFeeder:PROC
EXTERN UIFeeder_HandleClick:PROC
EXTERN UIFeeder_CheckSelection:PROC

FEEDER equ 0
INDEX equ 4
COLUMN equ 8

.code

UIFeeder_SetItemSelectionStub PROC
  mov     eax, [esp+08h]
  mov     dword ptr [UIFeeder_Current+FEEDER], eax

  mov     eax, [esp+0Ch]
  mov     dword ptr [UIFeeder_Current+INDEX], eax

  call    UIFeeder_SetItemSelection

  test    al, al
  jz      continue

  ret

continue:
  fld     dword ptr ds:[739FD0h]

  mov     eax, 4C25D6h
  jmp     eax
UIFeeder_SetItemSelectionStub ENDP

UIFeeder_GetItemTextStub PROC
  mov     eax, [esp+0Ch]
  mov     dword ptr [UIFeeder_Current+FEEDER], eax

  mov     eax, [esp+10h]
  mov     dword ptr [UIFeeder_Current+INDEX], eax

  mov     eax, [esp+14h]
  mov     dword ptr [UIFeeder_Current+COLUMN], eax

  call    UIFeeder_GetItemText

  test    eax, eax
  jz      continue

  push    ebx
  mov     ebx, [esp+4+28h]
  mov     dword ptr [ebx], 0
  pop     ebx
  ret

continue:
  push    ecx
  fld     dword ptr ds:[739FD0h]

  mov     eax, 4CE9E7h
  jmp     eax
UIFeeder_GetItemTextStub ENDP

UIFeeder_GetItemCountStub PROC
  mov     eax, [esp+8h]
  mov     dword ptr [UIFeeder_Current+FEEDER], eax

  call    UIFeeder_GetItemCount

  test    eax, eax
  jz      continue

  ret

continue:
  push    ecx
  fld     dword ptr ds:[739FD0h]

  mov     eax, 41A0D7h
  jmp     eax
UIFeeder_GetItemCountStub ENDP

UIFeeder_HandleKeyStub PROC
  mov     eax, [ebp+12Ch]
  mov     dword ptr [UIFeeder_Current+FEEDER], eax

  call    UIFeeder_CheckFeeder

  test    al, al
  jz      runDoubleClick

  push    ebp
  call    UIFeeder_HandleClick
  add     esp, 4

  test    al, al
  jnz     runDoubleClick

  ret

runDoubleClick:
  mov     eax, 635570h
  jmp     eax
UIFeeder_HandleKeyStub ENDP

UIFeeder_MouseEnterStub PROC
  mov     eax, [edi+12Ch]
  mov     dword ptr [UIFeeder_Current+FEEDER], eax

  call    UIFeeder_CheckFeeder

  test    al, al
  jnz     continue

  mov     [edi+130h], esi

continue:
  mov     eax, 639D75h
  jmp     eax
UIFeeder_MouseEnterStub ENDP

UIFeeder_MouseSelectStub PROC
  mov     eax, [esp+08h]
  mov     dword ptr [UIFeeder_Current+FEEDER], eax

  call    UIFeeder_CheckFeeder

  test    al, al
  jnz     continue

  mov     eax, 4C25D0h
  jmp     eax

continue:
  ret
UIFeeder_MouseSelectStub ENDP

UIFeeder_PlaySoundStub PROC
  mov     eax, [edi+12Ch]
  mov     dword ptr [UIFeeder_Current+FEEDER], eax

  call    UIFeeder_CheckFeeder

  test    al, al
  jnz     continue

  mov     eax, 685E10h
  jmp     eax

continue:
  ret
UIFeeder_PlaySoundStub ENDP

UIFeeder_CheckSelectionStub PROC
  mov     ecx, 6B5240h
  call    ecx

  push    eax
  call    UIFeeder_CheckSelection

  test    al, al
  jz      returnSafe

  pop     eax
  push    635A9Dh
  ret

returnSafe:
  pop     eax
  xor     eax, eax
  pop     ecx
  ret
UIFeeder_CheckSelectionStub ENDP

END
