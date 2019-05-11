#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//Given header files
#include "types.h"
#include "constants.h"

//Header files created by us
#include "user.h"
#include "parse.h"

int main(int argc, char* argv[]){
	setbuf(stdout, NULL);

	if(argc == 2 && strcmp(argv[1], "--help") == 0){
		user_help();
		exit(0);
	}

	if(argc != 6){
		printf("Usage: user id_account \"account_password\"\n"); 
		printf("			requested_delay operation_code\n");
		printf("			\"argument_1\"...\n");
		printf("Try user --help for more information\n.");
		exit(-1);
	}

	if(!check_number(argv[1])){
		printf("Account ID must be a positive integer.\n");
		exit(-2);
	}

	if(atoi(argv[1]) < 0 || atoi(argv[1]) > MAX_BANK_ACCOUNTS){
		printf("Account ID must be between 0 and %d", MAX_BANK_ACCOUNTS);
		exit(-3);
	}

	if(strlen(argv[1]) > 9){
		printf("Account ID cannot have more than 9 digits.\n");
		exit(-3);
	}

	if(strlen(argv[2]) > MAX_PASSWORD_LEN || strlen(argv[2]) < MIN_PASSWORD_LEN){
		printf("Password length must be between 8 to 20 characters.\n");
		exit(-4);
	}

	if(!check_number(argv[3])){
		printf("Operation delay must be a positive integer.\n");
		exit(-5);
	}

	if(!check_number(argv[4])){
		printf("Operation code must be a positive integer.\n");
		exit(-6);
	}

	if(atoi(argv[4]) < 0 || atoi(argv[4]) > 3){
		printf("Operation code must be between 0 and 3.\n");
		exit(-7);
	}

	if(atoi(argv[4]) == 1 || atoi(argv[4]) == 3){
		if(strlen(argv[5]) != 0){
			printf("Operations 1 and 3 require an empty argument list.\n");
			exit(-8);
		}
	}

	if(atoi(argv[4]) == 0 || atoi(argv[4]) == 2){
		if(strlen(argv[5]) == 0){
			printf("Operations 0 and 2 require a non empty argument list.\n");
			exit(-9);
		}
	}	

	client_inf* client_information = malloc(sizeof(client_inf));

	parse_client_inf(argv, client_information);

	char pid[6];
	sprintf(pid, "%d", getpid());
	char fifo_name[USER_FIFO_PATH_LEN];
	strcpy(fifo_name, USER_FIFO_PATH_PREFIX);
	strcat(fifo_name, pid);

	mkfifo(fifo_name, 0666);

	//printf("%d, %s, %d, %d, %s.\n", client_information->account_id, client_information->account_password, client_information->operation_delay, client_information->operation, client_information->operation_arguments);

	free_client_information(client_information);

	return 0;
}

void user_help(){
	printf("Usage: user id_account \"account_password\"\n"); 
	printf("			requested_delay operation_code\n");
	printf("			\"argument_1\"...\n");
	printf("Creates a homebanking server in your pc\n");
	printf("Example: user 1 \"my_pass\" 3000 0 \"\"\n\n");

	printf("id_account:\n"); 
	printf("	Id of the user account.\n\n");

	printf("account_password:\n");
	printf("	Password used to access the server.\n\n");

	printf("requested_delay:\n");
	printf("	Number of miliseconds to wait before start the operation.\n");
	printf("	This variable is used to test the synchronization of processes and threads.\n\n"); 

	printf("operation_code:\n");
	printf("	Code of the operation to execute.\n");
	printf("	Options: 0-Account Creation\n");
	printf("	         1-Balance Inquiry\n");
	printf("	         2-Money Transfer\n");
	printf("	         3-Server Shutdown\n");
	printf("	Operations 0 and 3 can only be executed by the administrator.\n\n");

	printf("list_of_arguments:\n");
	printf("	Arguments requested by the operation.\n");
}