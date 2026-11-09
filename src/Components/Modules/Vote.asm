.686
.model flat, C
option casemap:none

EXTERN Vote_GetVoteText:PROC

.code

Vote_GetVoteTextStub PROC
  push    ebx
  call    Vote_GetVoteText
  add     esp, 4
  ret
Vote_GetVoteTextStub ENDP

END
