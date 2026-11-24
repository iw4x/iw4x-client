option casemap:none

EXTERN Elevators_PM_Trace:PROC

.code

Elevators_PM_TraceStub PROC
  push    rbx
  sub     rsp, 30h

  mov     rbx, rcx

  mov     eax, [rsp+60h]
  mov     [rsp+20h], eax
  mov     eax, [rsp+68h]
  mov     [rsp+28h], eax

  call    qword ptr [rbp+r10*8]

  mov     rcx, rbx
  call    Elevators_PM_Trace

  add     rsp, 30h
  pop     rbx
  ret
Elevators_PM_TraceStub ENDP

END
