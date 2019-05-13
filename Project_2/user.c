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
#include "fifo.h"

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

	if(atoi(argv[4]) == 0){
		char *tmpstr = malloc(strlen(argv[5]));
		if(strstr(argv[5], " ") == NULL){
			printf("Last argument of operation 0 must be \"new_account_id balance password\".\n");
			free(tmpstr);
			exit(-10);
		}
		tmpstr=strstr(argv[5], " ") + 1;
		char *new_id = malloc(strlen(argv[5]));
		strncpy(new_id, argv[5], strstr(argv[5], " ") - argv[5]);
		if(!check_number(new_id)){
			printf("New account ID must be a positive integer between 1 and %d.\n", MAX_BANK_ACCOUNTS);
			free(tmpstr);
			free(new_id);
			exit(-10);
		}
		if(atoi(new_id)>MAX_BANK_ACCOUNTS || atoi(new_id)<1){
			printf("New account ID must be a positive integer between 1 and %d.\n", MAX_BANK_ACCOUNTS);
			free(tmpstr);
			free(new_id);
			exit(-10);
		}
		if(strstr(tmpstr, " ") == NULL){
			printf("Last argument of operation 0 must be \"new_account_id balance password\".\n");
			free(tmpstr);
			free(new_id);
			exit(-10);
		}
		char *balance = malloc(strlen(argv[5])-strlen(new_id));
		strncpy(balance, tmpstr, strstr(tmpstr, " ") - tmpstr);
		tmpstr = strstr(tmpstr, " ") + 1;
		if(!check_number(balance) || strtoul(balance, NULL, 10) < MIN_BALANCE || strtoul(balance, NULL, 10)>MAX_BALANCE){
			printf("New account balance must be a positive integer between 1 and %ld.\n", MAX_BALANCE);
			free(tmpstr);
			free(new_id);
			free(balance);
			exit(-10);
		}		
		char *password = malloc(strlen(argv[5]-strlen(new_id)-strlen(balance)));
		strncpy(password, tmpstr, strlen(tmpstr));
		free(tmpstr);
		free(new_id);
		free(balance);
		free(password);

	}

	if(atoi(argv[4]) == 2){
		char *tmpstr = malloc(strlen(argv[5]));
		if(strstr(argv[5], " ") == NULL){
			printf("Last argument of operation 2 must be \"destination_id amount\".\n");
			free(tmpstr);
			exit(-11);
		}
		tmpstr=strstr(argv[5], " ") + 1;
		char *dest_id = malloc(strlen(argv[5]));
		strncpy(dest_id, argv[5], strstr(argv[5], " ") - argv[5]);
		if(!check_number(dest_id)){
			printf("Destination account ID must be a positive integer between 1 and %d.\n", MAX_BANK_ACCOUNTS);
			free(tmpstr);
			free(dest_id);
			exit(-10);
		}
		char *amount = malloc(strlen(argv[5]-strlen(dest_id)));
		strncpy(amount, tmpstr, strlen(tmpstr));
		if(!check_number(amount) || strtoul(amount, NULL, 10) < MIN_BALANCE || strtoul(amount, NULL, 10)>MAX_BALANCE){
			printf("Amount must be a positive integer between 1 and %ld.\n", MAX_BALANCE);
			free(tmpstr);
			free(dest_id);
			free(amount);
			exit(-10);
		}
		free(tmpstr);
		free(dest_id);
		free(amount);
	}


	client_inf* client_information = malloc(sizeof(client_inf));

	parse_client_inf(argv, client_information);

	char pid[6];
	sprintf(pid, "%d", getpid());
	char fifo_name[USER_FIFO_PATH_LEN];
	strcpy(fifo_name, USER_FIFO_PATH_PREFIX);
	strcat(fifo_name, pid);

	if(mkfifo(fifo_name, 0666)){
		perror("mkfifo");
		exit(-1);
	}

	int srv_fifo = open(SERVER_FIFO_PATH, O_WRONLY | O_APPEND); //Opening server FIFO for writing
	if(srv_fifo < 0){
		perror("open server fifo");
		exit(-1);
	}

	write_srv_fifo(srv_fifo, client_information);

	if(close(srv_fifo)){
		perror("close server fifo");
		exit(-1);
	}

	int user_fifo = open(fifo_name, O_RDONLY);
	if(user_fifo < 0){
		perror("open user fifo");
		exit(-1);
	}

	//read_user_fifo()

	if(close(user_fifo)){
		perror("close user fifo");
		exit(-1);
	}

	free_client_information(client_information);

	//printf(fifo_name);

	if(unlink(fifo_name)){
		perror("unlink");
		exit(-1);
	}

	return 0;
}

void user_help(){
	printf("Usage: user id_account \"account_password\"\n"); 
	printf("			requested_delay operation_code\n");
	printf("			\"argument_1\"...\n");
	printf("Creates a homebanking server in your pc\n");
	printf("Example: user 1 \"my_pass\" 3000 1 \"\"\n\n");

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

void write_srv_fifo(int srv_fifo, client_inf* inf){
	char* client_string = malloc(sizeof(char)+3*sizeof(int)+strlen(inf->account_password)+strlen(inf->operation_arguments));	 
	sprintf(client_string, "%d %s %d %d %s\n", inf->account_id, inf->account_password, inf->operation_delay, inf->operation, inf->operation_arguments);
	
	if(write(srv_fifo, client_string, strlen(client_string)) < 0){
		perror("write");
		exit(-1);
	}

	free(client_string);	
}