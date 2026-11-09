.686
.model flat, C
option casemap:none

EXTERN Bullet_BGBulletRange:DWORD

EXTERN Bullet_BulletRicochet_Save:PROC

.code

Bullet_FireStub PROC
  push    eax
  mov     eax, Bullet_BGBulletRange
  fld     dword ptr [eax+10h]
  pop     eax

  push    440346h
  ret
Bullet_FireStub ENDP

Bullet_BulletRicochet_Stub PROC
  pushad
  push    dword ptr [esp+20h+0Ch]
  call    Bullet_BulletRicochet_Save
  add     esp, 4h
  popad

  sub     esp, 4Ch
  push    ebp
  mov     ebp, dword ptr [esp+60h]

  push    5D5B08h
  ret
Bullet_BulletRicochet_Stub ENDP

END
