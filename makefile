assmbler: assembler.o first_pass.o second_pass.o useful_funcs.o
	gcc -Wall -ansi -pedantic assembler.o first_pass.o second_pass.o useful_funcs.o -o assmbler
assembler.o: assembler.c assembler.h globals.h globals.h
	gcc -c -ansi -Wall assembler.c -o assembler.o 
first_pass.o: first_pass.c first_pass.h globals.h
	gcc -c -ansi -Wall first_pass.c -o first_pass.o
second_pass.o: second_pass.c second_pass.h globals.h
	gcc -c -ansi -Wall second_pass.c -o second_pass.o
useful_funcs.o: useful_funcs.c useful_funcs.h globals.h
	gcc -c -ansi -Wall useful_funcs.c -o useful_funcs.o




