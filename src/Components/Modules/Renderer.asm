.686
.model flat, C
option casemap:none

EXTERN Renderer_BackendFrameHandler:PROC
EXTERN Renderer_RB_RecoverDevice:PROC
EXTERN Renderer_PostVidRestart:PROC
EXTERN Renderer_R_TextureFromCodeError:PROC

.code

Renderer_BackendFrameStub PROC
  pushad
  call    Renderer_BackendFrameHandler
  popad

  mov     eax, dword ptr ds:[66E1BF0h]
  push    536A85h
  ret
Renderer_BackendFrameStub ENDP

Renderer_RB_SwapBuffers_PresentFailedStub PROC
  cmp     eax, 88760868h
  je      done

  push    eax
  push    eax
  call    Renderer_RB_RecoverDevice
  add     esp, 4h
  test    al, al
  pop     eax
  jnz     done

  push    536AABh
  ret

done:
  push    536AC1h
  ret
Renderer_RB_SwapBuffers_PresentFailedStub ENDP

Renderer_PostVidRestartStub PROC
  pushad
  call    Renderer_PostVidRestart
  popad

  push    4F84C0h
  ret
Renderer_PostVidRestartStub ENDP

Renderer_StoreGfxBufContextPtrStub1 PROC
  mov     eax, dword ptr [eax*4+66E600Ch]

  pushad

  push    edx
  push    dword ptr [esp+20h+24h+4h]
  push    eax
  call    Renderer_R_TextureFromCodeError
  add     esp, 0Ch

  popad

  push    54CAC1h
  ret
Renderer_StoreGfxBufContextPtrStub1 ENDP

Renderer_StoreGfxBufContextPtrStub2 PROC
  mov     edx, dword ptr [eax*4+66E600Ch]

  pushad
  push    eax
  push    ebx
  push    edx
  call    Renderer_R_TextureFromCodeError
  add     esp, 0Ch
  popad

  push    54CFA4h
  ret
Renderer_StoreGfxBufContextPtrStub2 ENDP

END
