assembler : assemble.o memory.o symbols.o
	gcc -g -ansi -Wall -pedantic assemble.o memory.o symbols.o -o assembler -lm

assemble.o : assemble.c assemble.h
	gcc -c -ansi -Wall -pedantic assemble.c -o assemble.o -lm

memory.o : memory.c memory.h
	gcc -c -ansi -Wall -pedantic memory.c -o memory.o

symbols.o : symbols.c symbols.h
	gcc -c -ansi -Wall -pedantic symbols.c -o symbols.o
