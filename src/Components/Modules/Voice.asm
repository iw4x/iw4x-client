EXTERN Voice_UI_Mute_player:PROC

.code

Voice_UI_Mute_Player_Stub PROC
  sub     rsp, 20h
  mov     edx, edi
  mov     ecx, eax
  call    Voice_UI_Mute_player
  add     rsp, 20h

  mov     rax, 140270555h
  jmp     rax
Voice_UI_Mute_Player_Stub ENDP

END
