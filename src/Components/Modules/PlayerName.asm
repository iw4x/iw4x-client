.686
.model flat, C
option casemap:none

EXTERN PlayerName_UserInfoCopy:PROC
EXTERN PlayerName_CopyClientNameCheck:PROC
EXTERN PlayerName_DropClient:PROC

.code

PlayerName_ClientCleanName PROC
  pushad

  push    dword ptr [esp+20h+4h]
  push    ecx
  push    edx

  call    PlayerName_UserInfoCopy
  add     esp, 0Ch

  popad

  ret
PlayerName_ClientCleanName ENDP

PlayerName_SV_UserinfoChangedStub PROC
  call    PlayerName_CopyClientNameCheck
  test    al, al

  jnz     returnSafe

  pushad

  push    edi
  call    PlayerName_DropClient
  add     esp, 4h

  popad

returnSafe:
  push    401988h
  ret
PlayerName_SV_UserinfoChangedStub ENDP

END
