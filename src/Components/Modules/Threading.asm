.686
.model flat, C
option casemap:none

EXTERN Threading_SVFrameWaitFunc:PROC
EXTERN Threading_ComFrameWait:PROC

.code

Threading_FrameEpilogueStub PROC
  pop     edi
  pop     esi
  pop     ebx
  mov     esp, ebp
  pop     ebp
  ret
Threading_FrameEpilogueStub ENDP

Threading_PacketEventStub PROC
  mov     eax, 49F0B0h
  call    eax

  mov     eax, 458160h
  jmp     eax
Threading_PacketEventStub ENDP

Threading_SVFrameWaitStub PROC
  pushad
  call    Threading_SVFrameWaitFunc
  popad

  push    4CD420h
  ret
Threading_SVFrameWaitStub ENDP

Threading_ComFrameWaitStub PROC
  push    ecx
  pushad

  push    edi
  call    Threading_ComFrameWait
  add     esp, 4

  mov     [esp+20h], eax
  popad
  pop     eax
  mov     ecx, eax

  mov     edx, dword ptr ds:[1AD7934h]
  cmp     byte ptr [edx+10h], 0

  push    47DDC1h
  ret
Threading_ComFrameWaitStub ENDP

END
