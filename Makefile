# Project Makefile

all: forensic

forensic: main.o
	gcc -o forensic main.o

main.o: main.c
	gcc -o main.o main.c -c -W -Wall -ansi -pedantic

clean:
	rm -rf *.o *~ forensic