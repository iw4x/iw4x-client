option casemap:none

EXTERN Auth_HasAccessToReservedSlot:BYTE

EXTERN Auth_ParseConnectData:PROC
EXTERN Auth_ProtocolMismatchMessage:PROC

.code

Auth_DirectConnectStub PROC
  mov     rdx, rcx
  mov     rcx, rsi
  jmp     Auth_ParseConnectData
Auth_DirectConnectStub ENDP

Auth_ProtocolMismatchStub PROC
  mov     ecx, edi
  jmp     Auth_ProtocolMismatchMessage
Auth_ProtocolMismatchStub ENDP

Auth_DirectConnectPrivateClientStub PROC
  xor     r14d, r14d
  cmp     Auth_HasAccessToReservedSlot, 0
  jne     done

  mov     rax, 14650B798h
  mov     rax, [rax]
  mov     r14d, [rax+10h]

done:
  ret
Auth_DirectConnectPrivateClientStub ENDP

END
