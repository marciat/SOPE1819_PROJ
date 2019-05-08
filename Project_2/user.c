#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Given header files
#include "types.h"

//Header files created by us
#include "user.h"

int main(int argc, char* argv[]){
	setbuf(stdout, NULL);

	if(argc == 2 && strcmp(argv[1], "--help") == 0){
		user_help();
		exit(0);
	}

	if(argc < 6){
		printf("Usage: user id_account \"account_password\"\n"); 
		printf("			requested_delay operation_code\n");
		printf("			\"argument_1\"...\n");
		printf("Try user --help for more information\n.");
		exit(-1);
	}


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