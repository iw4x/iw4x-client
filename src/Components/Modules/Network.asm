.686
.model flat, C
option casemap:none

EXTERN Network_CL_HandleCommand:PROC

WSAENETRESET equ 2744h

.code

Network_PacketErrorCheck PROC
  cmp     eax, 2746h
  jz      returnIgnore

  cmp     eax, WSAENETRESET
  jz      returnIgnore

  push    465325h
  ret

returnIgnore:
  push    4654C6h
  ret
Network_PacketErrorCheck ENDP

Network_CL_HandleCommandStub PROC
  lea     eax, [esp+0C54h]

  pushad

  push    ebp
  push    edi
  push    eax
  call    Network_CL_HandleCommand
  add     esp, 0Ch

  test    al, al

  popad

  jz      unhandled

  push    5A9E0Eh
  ret

unhandled:
  push    5AA719h
  ret
Network_CL_HandleCommandStub ENDP

END
