#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//#include <pthread.h>

//Given header files
#include "types.h"
#include "constants.h"

//Header files created by us
#include "server.h"
#include "parse.h"
#include "account.h"

pthread_t* threads;

void* bank_office(void* attr){
	attr = (void*)attr;
	return NULL;
}

int main(int argc, char* argv[]){
	setbuf(stdout, NULL);
	srand(time(NULL));

	if(argc == 2 && strcmp(argv[1], "--help") == 0){
		server_help();
		exit(0);
	}

	if(argc != 3){
		printf("Usage: server number_of_bank_offices \"admin_password\"\n");
		printf("Try server --help for more information\n.");
		exit(-1);
	}

	if(strlen(argv[2]) > MAX_PASSWORD_LEN || strlen(argv[2]) < MIN_PASSWORD_LEN){
		printf("Password length must be between 8 to 20 characters.\n");
		exit(-2);
	}

	if(!check_number(argv[1])){
		printf("First argument must be a positive integer.\n");
		exit(-3);
	}

	if(atoi(argv[1]) > MAX_BANK_OFFICES || atoi(argv[1]) < 1 || strlen(argv[1]) > 9){
		printf("Number of bank offices must be between 1 and 99.\n");
		exit(-4);
	}

	int num_bank_offices = atoi(argv[1]);

	bank_account_t *admin_account = create_admin_account(argv[2]);

	create_account_storage();

	print_account_to_file(admin_account);	
	print_account_to_file(admin_account);

	if(pthread_mutex_init(&save_account_mutex, NULL)){
		perror("pthread_mutex_init");
		exit(-1);
	}

	threads = malloc(sizeof(pthread_t)*num_bank_offices); 

	if(mkfifo(SERVER_FIFO_PATH, 0666)){
		perror("mkfifo");
		exit(-1);
	}

	for(int i = 1; i <= num_bank_offices; i++){
		pthread_t tid = i;
		if(pthread_create(&tid, NULL, bank_office, NULL)){
			perror("pthread_create");
			exit(-1);
		}
		threads[i-1] = tid;
	}

	for(int i = 0; i < num_bank_offices; i++){ //Joining all threads before exiting
		if(pthread_join(threads[i], NULL)){
			perror("pthread_join");
			exit(-1);
		}
	}

	free(threads);

	if(unlink(SERVER_FIFO_PATH)){
		perror("unlink");
		exit(-1);
	}

	return 0;
}

void server_help(){
	printf("Usage: server number_of_bank_offices \"admin_password\"\n");
	printf("Creates a homebanking server in your pc\n");
	printf("Example: server 10 \"bad_password\"\n\n");

	printf("number_of_bank_offices:\n"); 
	printf("	This argument is an integer that specifies the number of bank offices to create in the server.\n");
	printf("	Max number of bank offices: 99\n\n");

	printf("admin_password:\n");
	printf("	This argument represents the administrator password. This have to be between quotation marks.\n");
}