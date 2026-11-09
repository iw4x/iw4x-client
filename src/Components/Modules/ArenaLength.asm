.686
.model flat, C
option casemap:none

MAPNAME equ 0B00h

.code

ArenaLength_CompareMapNameStub1 PROC
  lea     eax, [esi+MAPNAME]
  push    eax
  push    ebx

  push    420725h
  ret
ArenaLength_CompareMapNameStub1 ENDP

ArenaLength_CompareMapNameStub2 PROC
  lea     eax, [edi+MAPNAME]
  push    eax
  push    edx

  push    49BD3Eh
  ret
ArenaLength_CompareMapNameStub2 ENDP

ArenaLength_CompareMapNameStub3 PROC
  lea     eax, [esi+MAPNAME]
  push    eax
  push    edx

  push    63279Eh
  ret
ArenaLength_CompareMapNameStub3 ENDP

ArenaLength_ReadVoteReferenceStub PROC
  lea     edi, [esi-MAPNAME]
  lea     edx, [eax+1]

  push    4064B8h
  ret
ArenaLength_ReadVoteReferenceStub ENDP

END
