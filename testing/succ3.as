.entry LENGTH
.extern W
MAIN:	mov @r3,LENGTH
LOOP:	jmp L1
	prn -5
	;This is a comment.
	bne LOOP
	sub @r1,@r4
	bne L3
L1:	inc K
.entry LOOP
    jmp W
END:	stop

STR:	.string "abcdef"
LENGTH:	.data 6,-9,15
K:	.data 22
.entry K

; this one should work perfectly!

ImJustAddingStuff: add 3, @r7
LetMeAddSomeMoreStuff: not @r0
IllJustAddOneMoreThing: clr LENGTH
;and a comment
	stop
