all: user server

user: user.c
	gcc -o user user.c -c -Wall -Wextra -Werror

server: server.o parse.o
	gcc -o server server.o parse.o -D_REENTRANT -lpthread

server.o: server.c
	gcc -o server.o server.c -D_REENTRANT -lpthread -c -Wall -Wextra -Werror

parse.o: parse.c
	gcc -o parse.o parse.c -c -Wall -Wextra -Werror

clean:
	rm -rf *.o *~ user server