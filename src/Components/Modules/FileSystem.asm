.686
.model flat, C
option casemap:none

EXTERN FileSystem_RegisterFolders:PROC

.code

FileSystem_StartupStub PROC
  pushad
  push    esi
  call    FileSystem_RegisterFolders
  pop     esi
  popad

  mov     edx, ds:[63D0CC0h]

  push    48264Dh
  ret
FileSystem_StartupStub ENDP

FileSystem_FS_FOpenFileReadForThread PROC
  sub     esp, 33Ch
  push    643276h
  ret
FileSystem_FS_FOpenFileReadForThread ENDP

END
