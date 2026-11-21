option casemap:none

EXTERN FileSystem_RegisterFolders:PROC
EXTERN FileSystem_VidRestarted:BYTE

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

FileSystem_VidRestartedStub PROC
  cmp     byte ptr [FileSystem_VidRestarted], 0
  jnz     done

  mov     r11, 140C5AE68h
  cmp     dword ptr [r11], 0

done:
  ret
FileSystem_VidRestartedStub ENDP

END
