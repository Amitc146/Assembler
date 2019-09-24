MAIN:	mov @r3,LENGTH
LOOP:	jmp
	prn -5
	bne LOOP
	sub @r1,@r4
	bne END
L1:	in K
	bne LOOP
END:	stop
STR:	.string "abcdef"
LENGTH:	.data 6,-9,15
K:	.data
