.686
.model flat, C
option casemap:none

.code

DB_FindXAssetDefaultHeaderInternal PROC
  push    eax
  pushad

  mov     eax, 5BB210h
  mov     edi, [esp+28h]
  call    eax

  mov     [esp+20h], eax
  popad
  pop     eax

  ret
DB_FindXAssetDefaultHeaderInternal ENDP

DB_FindXAssetEntry PROC
  push    eax
  pushad

  mov     edi, [esp+2Ch]
  push    edi

  mov     edi, [esp+2Ch]

  mov     eax, 5BB1B0h
  call    eax

  add     esp, 4h

  mov     [esp+20h], eax
  popad
  pop     eax

  ret
DB_FindXAssetEntry ENDP

DB_InsertPointerInternal PROC
  call    dword ptr [gameDB_InsertPointer]
  ret

gameDB_InsertPointer:
  dd      43B290h
DB_InsertPointerInternal ENDP

END
