option casemap:none

EXTERN Sound_ParseStreamHeader:PROC

.code

Sound_ParseStreamHeaderStub PROC
  mov     r9, rbx
  jmp     Sound_ParseStreamHeader
Sound_ParseStreamHeaderStub ENDP

END
