.686
.model flat, C
option casemap:none

EXTERN VisionFile_LoadVisionSettingsFromBuffer:PROC

.code

VisionFile_LoadVisionSettingsFromBuffer_Stub PROC
  push    eax
  pushad

  push    dword ptr [esp+24h+8h]
  push    ebx
  push    dword ptr [esp+24h+0Ch]
  call    VisionFile_LoadVisionSettingsFromBuffer
  add     esp, 0Ch

  mov     [esp+20h], eax
  popad
  pop     eax

  ret
VisionFile_LoadVisionSettingsFromBuffer_Stub ENDP

END
