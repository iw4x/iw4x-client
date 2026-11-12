option casemap:none

EXTERN AssetHandler_ShouldOverrideAsset:PROC

.code

AssetHandler_OverrideAssetStub PROC
  push    rcx
  push    r8
  sub     rsp, 20h

  mov     ecx, ebp
  call    AssetHandler_ShouldOverrideAsset

  add     rsp, 20h
  pop     r8
  pop     rcx

  test    al, al
  jnz     overrideAsset

  mov     rax, 14012E988h
  jmp     rax

overrideAsset:
  mov     rax, 14012E964h
  jmp     rax
AssetHandler_OverrideAssetStub ENDP

END
