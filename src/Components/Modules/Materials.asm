option casemap:none

EXTERN Materials_WriteDeathMessageIcon:PROC

.code

Materials_ImageVersionCheck PROC
  movzx   eax, byte ptr [rbp-16h]
  cmp     eax, 8
  je      versionOk
  cmp     eax, 9
  je      versionOk

  mov     rax, 14006A1D6h
  jmp     rax

versionOk:
  mov     rax, 14006A1FEh
  jmp     rax
Materials_ImageVersionCheck ENDP

Materials_DeathMessageStub PROC
  sub     rsp, 28h

  lea     rcx, [rsp+70h]
  mov     edx, ebx
  mov     r8, rax
  call    Materials_WriteDeathMessageIcon

  add     rsp, 28h

  mov     edx, eax
  movzx   ecx, byte ptr [rdi]
  ret
Materials_DeathMessageStub ENDP

END
