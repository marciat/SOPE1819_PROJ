# Project Makefile

all: forensic

forensic: forensic.o
	gcc -o forensic forensic.o

forensic.o: forensic.c
	gcc -o forensic.o forensic.c -c -Wall -pedantic

clean:
	rm -rf *.o *~ forensic
