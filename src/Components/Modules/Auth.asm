.686
.model flat, C
option casemap:none

EXTERN Auth_HasAccessToReservedSlot:BYTE

EXTERN Auth_ParseConnectData:PROC
EXTERN Auth_ProtocolMismatchMessage:PROC

.code

Auth_DirectConnectStub PROC
  pushad
  lea     eax, [esp+20h]
  push    eax
  push    esi
  call    Auth_ParseConnectData
  pop     esi
  pop     eax
  popad

  push    6265FEh
  ret
Auth_DirectConnectStub ENDP

Auth_ProtocolMismatchStub PROC
  push    esi
  call    Auth_ProtocolMismatchMessage
  add     esp, 4
  ret
Auth_ProtocolMismatchStub ENDP

Auth_DirectConnectPrivateClientStub PROC
  push    eax

  mov     al, Auth_HasAccessToReservedSlot
  test    al, al

  pop     eax

  je      noAccess

  xor     eax, eax
  jmp     safeContinue

noAccess:
  mov     eax, dword ptr [edx+10h]

safeContinue:
  add     esp, 0Ch

  push    460FB3h
  ret
Auth_DirectConnectPrivateClientStub ENDP

END
