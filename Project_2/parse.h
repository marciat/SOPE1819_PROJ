//Header file that declares functions and data structures used to parse data

#ifndef _PARSE_H
#define _PARSE_H

typedef struct {
	int num_bank_offices;
	char* admin_password;
	char* salt;
} server_inf;

typedef struct {
	int id_conta;
	char* password_conta;
	int atraso_operacao;
	int operacao_realizar;
	char* args_operacao;
} client_inf;

void parse_server_inf(char* argv[], server_inf* inf);

void parse_client_inf(char* argv[], client_inf* inf);

void salt_generator(char* salt);

void rem_quot(char* password, char* pass_with_quot);

int check_number(char* number_bank_offices);

void free_server_information(server_inf* server_information);

void free_client_information(client_inf* client_information);

#define READ 0
#define WRITE 1

#endif