.686
.model flat, C
option casemap:none

EXTERN Logger_PipeCallback:DWORD

EXTERN Logger_PrintMessageSentry:PROC
EXTERN Logger_PrintMessagePipe:PROC
EXTERN Logger_NetworkLog:PROC
EXTERN Logger_RedirectOSPath:PROC

.code

Logger_PrintMessage_Stub PROC
  pushad

  push    dword ptr [esp+28h]
  call    Logger_PrintMessageSentry
  add     esp, 4h

  popad

  mov     eax, Logger_PipeCallback
  test    eax, eax
  jz      returnPrint

  pushad

  push    dword ptr [esp+28h]
  call    Logger_PrintMessagePipe
  add     esp, 4h

  popad
  ret

returnPrint:
  pushad

  push    0
  push    dword ptr [esp+2Ch]
  call    Logger_NetworkLog
  add     esp, 8h

  popad

  push    esi
  mov     esi, [esp+0Ch]

  push    4AA835h
  ret
Logger_PrintMessage_Stub ENDP

Logger_BuildOSPath_Stub PROC
  pushad

  push    dword ptr [esp+20h+8h]
  push    dword ptr [esp+20h+10h]
  call    Logger_RedirectOSPath
  add     esp, 8h

  popad

  mov     eax, [esp+8h]
  push    ebp
  push    esi
  mov     esi, [esp+0Ch]

  push    64213Fh
  ret
Logger_BuildOSPath_Stub ENDP

END
