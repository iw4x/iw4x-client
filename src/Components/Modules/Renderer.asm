EXTERN Renderer_BackendFrameHandler:PROC
EXTERN Renderer_RB_RecoverDevice:PROC
EXTERN Renderer_R_TextureFromCodeError:PROC

.code

Renderer_BackendFrameStub PROC
  sub     rsp, 20h
  call    Renderer_BackendFrameHandler
  add     rsp, 20h

  mov     rax, 148CC7FC4h
  movsxd  rax, dword ptr [rax]

  mov     rcx, 14004FD07h
  jmp     rcx
Renderer_BackendFrameStub ENDP

Renderer_RB_SwapBuffers_PresentFailedStub PROC
  cmp     eax, 88760868h
  je      done

  push    rax
  sub     rsp, 28h
  mov     ecx, eax
  call    Renderer_RB_RecoverDevice
  add     rsp, 28h
  test    al, al
  pop     rax
  jnz     done

  mov     rcx, 14004FD54h
  jmp     rcx

done:
  mov     rcx, 14004FD6Ch
  jmp     rcx
Renderer_RB_SwapBuffers_PresentFailedStub ENDP

Renderer_TextureFromCodeErrorStub1 PROC
  mov     rcx, r8
  mov     rdx, qword ptr [rdi+8]
  mov     r8d, dword ptr [rbx+6]
  jmp     Renderer_R_TextureFromCodeError
Renderer_TextureFromCodeErrorStub1 ENDP

Renderer_TextureFromCodeErrorStub2 PROC
  mov     rcx, r8
  mov     rdx, qword ptr [r14+8]
  mov     r8d, dword ptr [rdi+6]
  jmp     Renderer_R_TextureFromCodeError
Renderer_TextureFromCodeErrorStub2 ENDP

END
