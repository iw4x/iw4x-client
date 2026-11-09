.686
.model flat, C
option casemap:none

EXTERN ModelSurfs_SetBuffer:PROC
EXTERN ModelSurfs_GetBuffer:PROC

.code

ModelSurfs_GetIndexBufferStub PROC
  mov     eax, [esp+4h]
  cmp     al, 0FFh

  jne     returnSafe

  jmp     ModelSurfs_SetBuffer

returnSafe:
  movzx   eax, byte ptr [esp+4h]
  mov     edx, 4B4DE5h
  jmp     edx
ModelSurfs_GetIndexBufferStub ENDP

ModelSurfs_GetIndexBufferStub2 PROC
  mov     eax, [esp+4h]
  cmp     al, 0FFh

  jne     returnSafe

  mov     eax, [edi+0Ch]
  push    eax
  call    ModelSurfs_GetBuffer
  add     esp, 4h
  ret

returnSafe:
  mov     eax, 4FDC20h
  jmp     eax
ModelSurfs_GetIndexBufferStub2 ENDP

ModelSurfs_GetIndexBaseStub PROC
  mov     eax, [esp+4h]
  cmp     al, 0FFh

  jne     returnSafe

  xor     eax, eax
  ret

returnSafe:
  mov     eax, 48C5F0h
  jmp     eax
ModelSurfs_GetIndexBaseStub ENDP

ModelSurfs_GetVertexBufferStub PROC
  mov     eax, [esp+4h]
  cmp     al, 0FFh

  jne     returnSafe

  jmp     ModelSurfs_SetBuffer

returnSafe:
  movzx   eax, byte ptr [esp+4h]
  mov     edx, 5BC055h
  jmp     edx
ModelSurfs_GetVertexBufferStub ENDP

END
