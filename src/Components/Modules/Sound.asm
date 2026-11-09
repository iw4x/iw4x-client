.686
.model flat, C
option casemap:none

.const

bufferError db "Error: Failed to create DirectSound play buffer", 0Ah, 0

.code

Sound_Init PROC
  cmp     byte ptr ds:[1AA4908h], 0
  jnz     proceed
  xor     eax, eax
  ret

proceed:
  push    esi
  mov     eax, 454E40h
  call    eax
  mov     esi, eax

  test    esi, esi
  jnz     ok
  pop     esi
  ret

ok:
  push    edi

  mov     eax, dword ptr ds:[1AA490Ch]
  test    eax, eax
  jz      fail_e

  mov     eax, dword ptr [eax]
  test    eax, eax
  jz      fail_e

  mov     edx, dword ptr ds:[79B174h]
  push    edx
  mov     dword ptr [esi+8], edx
  lea     edi, [esi+4]
  push    edi
  mov     ecx, dword ptr [esi+38h]
  mov     eax, dword ptr [esi+2Ch]

  mov     edx, 6015D0h
  call    edx
  add     esp, 8

  test    eax, eax
  jge     success

  push    OFFSET bufferError
  push    9
  mov     eax, 402500h
  call    eax
  add     esp, 8

  mov     eax, dword ptr [edi]
  test    eax, eax
  jz      fail_c

  mov     ecx, dword ptr [eax]
  mov     edx, dword ptr [ecx+8]
  push    eax
  call    edx

fail_c:
  mov     dword ptr [edi], 0
fail_e:
  pop     edi
  xor     eax, eax
  pop     esi
  ret

success:
  pop     edi
  mov     eax, esi
  pop     esi
  ret
Sound_Init ENDP

Sound_Loop PROC
  test    eax, eax
  jz      fail_init

  mov     dword ptr [esi], eax
  add     esi, 4

  push    4A23EBh
  ret

fail_init:
  xor     al, al
  pop     esi
  add     esp, 8
  ret
Sound_Loop ENDP

END
