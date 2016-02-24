all: slot

slot: slot.o
	gcc slot.o -o slot
	
slot.o: slot.c
	gcc -Wall -std=c11 -pedantic -ansi -m64 -c slot.c

clean:
	rm *o slot
