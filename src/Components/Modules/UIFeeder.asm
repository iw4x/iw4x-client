EXTERN UIFeeder_Current:DWORD

EXTERN UIFeeder_CheckFeeder:PROC
EXTERN UIFeeder_CheckSelection:PROC

.code

UIFeeder_MouseEnterStub PROC
  mov     eax, [rdi+180h]
  mov     dword ptr [UIFeeder_Current], eax

  push    rax
  sub     rsp, 28h
  call    UIFeeder_CheckFeeder
  add     rsp, 28h
  mov     ecx, eax
  pop     rax

  test    cl, cl
  jnz     continue

  mov     [rdi+rsi*4+184h], ebp

continue:
  mov     rcx, 14025E6B8h
  jmp     rcx
UIFeeder_MouseEnterStub ENDP

UIFeeder_PlaySoundStub PROC
  mov     eax, [rdi+180h]
  mov     dword ptr [UIFeeder_Current], eax

  push    rcx
  push    rdx
  push    r8
  sub     rsp, 20h
  call    UIFeeder_CheckFeeder
  add     rsp, 20h
  pop     r8
  pop     rdx
  pop     rcx

  test    al, al
  jnz     skip

  mov     rax, 140245250h
  jmp     rax

skip:
  ret
UIFeeder_PlaySoundStub ENDP

UIFeeder_CheckSelectionStub PROC
  cvttss2si eax, dword ptr [rsi+180h]

  push    rax
  sub     rsp, 28h
  mov     ecx, eax
  call    UIFeeder_CheckSelection
  add     rsp, 28h
  mov     ecx, eax
  pop     rax

  test    ecx, ecx
  jz      unselected

  mov     rcx, 14025F61Ch
  jmp     rcx

unselected:
  mov     rcx, 14025F651h
  jmp     rcx
UIFeeder_CheckSelectionStub ENDP

END
