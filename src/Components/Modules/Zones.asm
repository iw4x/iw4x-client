.686
.model flat, C
option casemap:none

EXTERN Zones_ZoneVersion:DWORD
EXTERN Zones_currentAssetType:DWORD
EXTERN Zones_previousAssetType:DWORD

EXTERN Zones_LoadXModelLodInfo:PROC
EXTERN Zones_ImpactFxArrayCount:PROC
EXTERN Zones_CheckGameMapSp:PROC
EXTERN Zones_PathDataSize:PROC

.code

Zones_LoadXModelLodInfoStub PROC
  pushad

  push    edi
  call    Zones_LoadXModelLodInfo
  add     esp, 4h

  popad

  push    4EA703h
  push    40D7A0h
  ret
Zones_LoadXModelLodInfoStub ENDP

Zones_LoadImpactFxArray PROC
  push    edi
  pushad

  push    edi
  call    Zones_ImpactFxArrayCount
  pop     edi

  mov     [esp+20h], eax

  popad
  pop     edi

  push    4447E0h
  ret
Zones_LoadImpactFxArray ENDP

Zones_GameMapSpPatchStub PROC
  pushad

  push    eax
  call    Zones_CheckGameMapSp
  add     esp, 4h

  test    al, al
  jnz     returnSafe

  popad
  push    4189AEh
  ret

returnSafe:
  popad
  push    41899Dh
  ret
Zones_GameMapSpPatchStub ENDP

Zones_LoadPathDataConstant PROC
  push    esi
  pushad

  call    Zones_PathDataSize

  add     [esp+20h], eax

  popad
  pop     esi

  push    4D6A4Dh
  ret
Zones_LoadPathDataConstant ENDP

Zones_LoadXModelColSurfPtr PROC
  cmp     dword ptr [eax], 0
  je      dontLoadAssetData

  cmp     dword ptr [eax], 0FFFFFFFFh
  je      loadAssetData

  cmp     Zones_ZoneVersion, 446
  jl      loadAssetData

  pushad
  push    eax
  call    dword ptr [gameDB_ConvertOffsetToPointer_Address]
  add     esp, 4
  popad

dontLoadAssetData:
  push    4C870Eh
  ret

loadAssetData:
  push    4C86DDh
  ret

gameDB_ConvertOffsetToPointer_Address:
  dd      4A82B0h
Zones_LoadXModelColSurfPtr ENDP

Zones_FS_FOpenFileReadForThreadOriginal PROC
  sub     esp, 33Ch

  push    643276h
  ret
Zones_FS_FOpenFileReadForThreadOriginal ENDP

Zones_FS_ReadOriginal PROC
  push    ecx
  mov     eax, [esp+10h]

  push    4A04C5h
  ret
Zones_FS_ReadOriginal ENDP

Zones_FS_FCloseFileOriginal PROC
  mov     eax, [esp+4]
  push    esi

  push    462005h
  ret
Zones_FS_FCloseFileOriginal ENDP

Zones_FS_SeekOriginal PROC
  push    esi
  mov     esi, [esp+8]

  push    4A63D5h
  ret
Zones_FS_SeekOriginal ENDP

Zones_LoadMapTriggersModelPointer PROC
  cmp     dword ptr [edx+4], 0
  je      dontLoadAssetData

  cmp     dword ptr [edx+4], 0FFFFFFFFh
  je      loadAssetData

  cmp     Zones_ZoneVersion, 446
  jl      loadAssetData

  pushad
  push    eax
  call    dword ptr [gameDB_ConvertOffsetToPointer_Address]
  add     esp, 4
  popad

dontLoadAssetData:
  push    43CBF3h
  ret

loadAssetData:
  push    43CBC1h
  ret

gameDB_ConvertOffsetToPointer_Address:
  dd      4A82B0h
Zones_LoadMapTriggersModelPointer ENDP

Zones_LoadMapTriggersHullPointer PROC
  cmp     dword ptr [eax+0Ch], 0
  je      dontLoadAssetData

  cmp     dword ptr [eax+0Ch], 0FFFFFFFFh
  je      loadAssetData

  cmp     Zones_ZoneVersion, 446
  jl      loadAssetData

  pushad
  push    eax
  call    dword ptr [gameDB_ConvertOffsetToPointer_Address]
  add     esp, 4
  popad

dontLoadAssetData:
  push    43CC2Eh
  ret

loadAssetData:
  push    43CBFEh
  ret

gameDB_ConvertOffsetToPointer_Address:
  dd      4A82B0h
Zones_LoadMapTriggersHullPointer ENDP

Zones_LoadMapTriggersSlabPointer PROC
  cmp     dword ptr [eax+14h], 0
  je      dontLoadAssetData

  cmp     dword ptr [eax+14h], 0FFFFFFFFh
  je      loadAssetData

  cmp     Zones_ZoneVersion, 446
  jl      loadAssetData

  pushad
  push    eax
  call    dword ptr [gameDB_ConvertOffsetToPointer_Address]
  add     esp, 4
  popad

dontLoadAssetData:
  push    43CC6Dh
  ret

loadAssetData:
  push    43CC39h
  ret

gameDB_ConvertOffsetToPointer_Address:
  dd      4A82B0h
Zones_LoadMapTriggersSlabPointer ENDP

Zones_GetCurrentAssetTypeStub PROC
  pushad
  mov     eax, Zones_currentAssetType
  mov     Zones_previousAssetType, eax
  popad

  mov     eax, dword ptr ds:[112AA54h]
  mov     eax, [eax]

  mov     Zones_currentAssetType, eax

  push    418847h
  ret
Zones_GetCurrentAssetTypeStub ENDP

END
