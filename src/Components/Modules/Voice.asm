.686
.model flat, C
option casemap:none

EXTERN Voice_UI_Mute_player:PROC

.code

Voice_UI_Mute_Player_Stub PROC
  push    eax
  call    Voice_UI_Mute_player
  add     esp, 8

  pop     edi
  pop     esi
  add     esp, 0C00h
  ret
Voice_UI_Mute_Player_Stub ENDP

END
