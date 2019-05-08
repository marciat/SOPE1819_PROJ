//Header file that declares functions and data structures used to parse data

#ifndef _PARSE_H
#define _PARSE_H

typedef struct {
	int num_bank_offices;
	char* admin_password;
	char* salt;
} server_inf;

void parse_server_inf(char* argv[], server_inf* inf);

void salt_generator(char* salt);

void rem_quot(char* password, char* pass_with_quot);

void free_server_information(server_inf* server_information);

#define READ 0
#define WRITE 1

#endif