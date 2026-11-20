option casemap:none

EXTERN FileSystem_RegisterFolders:PROC

.code

FileSystem_StartupStub PROC
  sub     rsp, 20h
  call    FileSystem_RegisterFolders
  add     rsp, 20h

  mov     rax, 146642A08h
  mov     rax, [rax]

  mov     rcx, 140277073h
  jmp     rcx
FileSystem_StartupStub ENDP

END
