# Project Makefile

all: forensic process_data

forensic: forensic.c 
	gcc -o forensic forensic.c -c -Wall -pedantic

#forensic: forensic.o
#	gcc -o forensic forensic.o

#forensic.o: forensic.c
#	gcc -o forensic.o forensic.c -c -Wall -pedantic

process_data: process_data.c 
	gcc -o process_data process_data.c -c -Wall -pedantic

#process_data: process_data.o
#	gcc -o process_data process_data.o

#process_data.o: process_data.c 
#	gcc -o process_data.o process_data.c -c -Wall -pedantic

clean:
	rm -rf *.o *~ forensic process_data
