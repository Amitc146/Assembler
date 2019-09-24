	mov @r3,LENGTH
	bne L3
Y:	inc K
.entry LOOP
    clr W
END:	stop

STR:	.string "abcdef"
LENGTH:	.data 6,-9,15
K:	.data 22
.entry K

; I believe that one should work
