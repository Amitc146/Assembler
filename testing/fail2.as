.entry LENGTH
.extern W
MAIN:	mov @r3,LENGTH
LOOP:	jmp L1
	prn -5
	bne W
	sub @r1,@r4
	bne L3
L1:	inc K
.entry LOOP
	jmp W
END:	sto
STR:	.string "abcdef"
LENGTH:	.data 6,-9,15
K:	.data 22
.extern L3

;That one should fail..
; Just because i removed the p from stop
; so it should print an error for line 12
