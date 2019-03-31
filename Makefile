# Project Makefile

all: forensic

forensic: forensic.o process_data.o parse_data.o
	gcc -o forensic forensic.o process_data.o parse_data.o

forensic.o: forensic.c
	gcc -o forensic.o forensic.c -c -Wall

process_data.o: process_data.c 
	gcc -o process_data.o process_data.c -c -Wall

parse_data.o: parse_data.c
	gcc -o parse_data.o parse_data.c -c -Wall

clean:
	rm -rf *.o *~ forensic
