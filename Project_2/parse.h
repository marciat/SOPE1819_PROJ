//Header file that declares functions and data structures used to parse data

#ifndef _PARSE_H
#define _PARSE_H

typedef struct {
	int account_id;
	char* account_password;
	int operation_delay;
	int operation;
	char* operation_arguments;
} client_inf;

void parse_client_inf(char* argv[], client_inf* inf);

int check_number(char* number_bank_offices);

void free_client_information(client_inf* client_information);

#define READ 0
#define WRITE 1

#endif