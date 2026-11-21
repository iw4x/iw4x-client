EXTERN VisionFile_LoadVisionSettingsFromBuffer:PROC

.code

VisionFile_LoadVisionSettingsFromBuffer_Stub PROC
  sub     rsp, 20h
  mov     r8, r15
  lea     rdx, [rbp-9]
  mov     rcx, rbx
  call    VisionFile_LoadVisionSettingsFromBuffer
  add     rsp, 20h

  mov     rax, 1400BFEEBh
  jmp     rax
VisionFile_LoadVisionSettingsFromBuffer_Stub ENDP

END
