EXTERN Rumble_PlayNoteMappedRumbleAliases:PROC
EXTERN Rumble_MeleeRumble_Hook:PROC

.code

Rumble_PlayNoteMappedSoundAliases_Stub PROC
  push    rax
  push    rcx
  sub     rsp, 20h

  mov     r8, rdi
  mov     rdx, rcx
  mov     ecx, ebp
  call    Rumble_PlayNoteMappedRumbleAliases

  add     rsp, 20h
  pop     rcx
  pop     rax

  mov     rax, [rdi+30h]
  cmp     word ptr [rax], 0

  mov     rdx, 1400C7BD0h
  jmp     rdx
Rumble_PlayNoteMappedSoundAliases_Stub ENDP

Rumble_MeleeRumble_Stub PROC
  sub     rsp, 20h

  mov     rdx, r15
  mov     rcx, rbx
  call    Rumble_MeleeRumble_Hook

  add     rsp, 20h

  cmp     qword ptr [rbx+158h], 0

  mov     rax, 1401894C6h
  jmp     rax
Rumble_MeleeRumble_Stub ENDP

END
