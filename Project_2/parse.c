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
#include "types.h"

void parse_client_inf(char* argv[],	tlv_request_t* request){
	req_header_t header;
	req_value_t value;

	char clear_password[MAX_PASSWORD_LEN+1] = {'\0'};
	strcpy(header.password, clear_password);

	request->type = atoi(argv[4]);
	request->length = 0;
	
	header.pid = getpid();
	header.account_id = atoi(argv[1]);
	strcpy(header.password, argv[2]);
	header.op_delay_ms = strtoul(argv[3], NULL, 10);

	if(atoi(argv[4]) == 0){
    	req_create_account_t create;
		char *token;
		const char s[2] = " ";
		char *new_id = malloc(50);
		char *balance = malloc(50);
		char *password = malloc(50);
		strcpy(create.password, clear_password);

		token = strtok(argv[5], s);
		strcpy(new_id, token);

		token = strtok(NULL, s);
		strcpy(balance, token);

		token = strtok(NULL, s);
		strcpy(password, token);

		create.account_id = atoi(new_id);
		create.balance = strtoul(balance, NULL, 10);
		strcpy(create.password, password);
		value.create = create;
		free(new_id);
		free(balance);
		free(password);
		request->length+= sizeof(req_create_account_t);
	}

	if(atoi(argv[4]) == 2){	
    	req_transfer_t transfer;
		char *token;
		const char s[2] = " ";
		char *dest_id = malloc(50);
		char *amount = malloc(50);

		token = strtok(argv[5], s);
		strcpy(dest_id, token);
		token = strtok(NULL, s);
		strcpy(amount, token);

		transfer.account_id = atoi(dest_id);
		transfer.amount = strtoul(amount,NULL,10);
		value.transfer = transfer;

		free(dest_id);
		free(amount);

		request->length += sizeof(req_transfer_t);

	}

	value.header = header;
	request->value = value;
	request->length += sizeof(req_header_t);
}

int check_number(char* number){

	for(int i = 0; i < (int)strlen(number); i++){
		if(!isdigit(number[i])){
			return 0;
		}
	}
	return 1;
}

