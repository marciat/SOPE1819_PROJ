all: user server

user: user.o parse.o
	gcc -o user user.o parse.o -Wall -Wextra -Werror

server: server.o parse.o account.o reqqueue.o
	gcc -o server server.o parse.o account.o reqqueue.o -D_REENTRANT -lpthread

user.o: user.c
	gcc -o user.o user.c -c -Wall -Wextra -Werror

server.o: server.c
	gcc -o server.o server.c -D_REENTRANT -lpthread -c -Wall -Wextra -Werror

parse.o: parse.c
	gcc -o parse.o parse.c -c -Wall -Wextra -Werror

account.o: account.c
	gcc -o account.o account.c -c -Wall -Wextra -Werror

reqqueue.o: reqqueue.c
	gcc -o reqqueue.o reqqueue.c -c -Wall -Wextra -Werror

clean:
	rm -rf *.o *~ user server