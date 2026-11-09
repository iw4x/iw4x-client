.686
.model flat, C
option casemap:none

EXTERN AssetHandler_ShouldSearchTempAssets:BYTE

EXTERN AssetHandler_HasThreadBypass:PROC
EXTERN AssetHandler_FindAsset:PROC
EXTERN AssetHandler_FindTemporaryAsset:PROC
EXTERN AssetHandler_IsAssetEligible:PROC
EXTERN AssetHandler_ShouldOverrideAsset:PROC
EXTERN AssetHandler_StoreEmptyAsset:PROC

.code

AssetHandler_FindAssetStub PROC
  push    ecx
  push    ebx
  push    ebp
  push    esi
  push    edi

  push    eax
  pushad

  call    AssetHandler_HasThreadBypass

  mov     [esp+20h], eax
  popad
  pop     eax

  test    al, al
  jnz     checkTempAssets

  mov     ecx, [esp+18h]
  mov     ebx, [esp+1Ch]

  push    eax
  pushad

  push    ebx
  push    ecx

  call    AssetHandler_FindAsset

  add     esp, 8h

  mov     [esp+20h], eax
  popad
  pop     eax

  test    eax, eax
  jnz     finishFound

checkTempAssets:
  mov     al, AssetHandler_ShouldSearchTempAssets
  test    eax, eax
  jz      finishOriginal

  mov     ecx, [esp+18h]
  mov     ebx, [esp+1Ch]

  push    ebx
  push    ecx

  call    AssetHandler_FindTemporaryAsset

  add     esp, 8h

  test    eax, eax
  jnz     finishFound

finishOriginal:
  mov     ebx, dword ptr ds:[6D7190h]
  mov     eax, 40793Bh
  jmp     eax

finishFound:
  pop     edi
  pop     esi
  pop     ebp
  pop     ebx
  pop     ecx
  ret
AssetHandler_FindAssetStub ENDP

AssetHandler_AddAssetStub PROC
  push    eax
  pushad

  push    dword ptr [esp+2Ch]
  push    dword ptr [esp+2Ch]
  call    AssetHandler_IsAssetEligible
  add     esp, 8h

  mov     [esp+20h], eax
  popad
  pop     eax

  test    al, al
  jz      doNotLoad

  mov     eax, [esp+8h]
  sub     esp, 14h
  mov     ecx, 5BB657h
  jmp     ecx

doNotLoad:
  mov     eax, [esp+8h]
  ret
AssetHandler_AddAssetStub ENDP

AssetHandler_OverrideAssetStub PROC
  sub     esp, 4
  pushad
  push    ebp
  call    AssetHandler_ShouldOverrideAsset
  add     esp, 4

  mov     [esp+20h], eax
  popad

  cmp     byte ptr [esp], 0
  lea     esp, [esp+4]
  jnz     overrideAsset

  push    5BB8A4h
  ret

overrideAsset:
  push    5BB876h
  ret
AssetHandler_OverrideAssetStub ENDP

AssetHandler_StoreEmptyAssetStub PROC
  pushad
  push    ebx
  push    eax

  call    AssetHandler_StoreEmptyAsset

  pop     eax
  pop     ebx
  popad

  push    5BB290h
  ret
AssetHandler_StoreEmptyAssetStub ENDP

END
