#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <wait.h>

#include "parse.h"
#include "constants.h"

void parse_server_inf(char* argv[], server_inf* inf){
	inf->num_bank_offices = atoi(argv[1]);
	size_t password_length = sizeof(argv[2])*sizeof(char);
	inf->admin_password = malloc(password_length);
	inf->admin_password = memcpy(inf->admin_password, argv[2], password_length);
}

void parse_client_inf(char* argv[], client_inf* inf){
	inf->account_password = malloc(MAX_PASSWORD_LEN+1);
	inf->operation_arguments = malloc(100);

	inf->account_id = atoi(argv[1]);
	strcpy(inf->account_password, argv[2]);
	inf->operation_delay = atoi(argv[3]);
	inf->operation = atoi(argv[4]);
	strcpy(inf->operation_arguments, argv[5]);
}

void free_server_information(server_inf* server_information){
	free(server_information->admin_password);
	//free(server_information->salt);
	free(server_information);
}

void free_client_information(client_inf* client_information){
	free(client_information->account_password);
	free(client_information->operation_arguments);
	free(client_information);
}

int check_number(char* number_bank_offices){

	for(int i = 0; i < (int)strlen(number_bank_offices); i++){
		if(!isdigit(number_bank_offices[i])){
			return 0;
		}
	}
	return 1;
}

