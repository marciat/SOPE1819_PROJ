//Header file that declares functions used with accounts
#ifndef _ACCOUNT_H
#define _ACCOUNT_H

#include <pthread.h>

#include "parse.h"
#include "types.h"

bank_account_t* create_client_account(client_inf* client_information);

bank_account_t* create_admin_account(char* admin_password);

uint32_t check_balance(uint32_t account_id, char* password);

void money_transfer(uint32_t account_id, char* password, uint32_t new_account_id, uint32_t balance);

void salt_generator(char* salt);

void create_account_storage();

void delete_account_storage();

void print_account_to_file(bank_account_t *account);

void get_hash(char* password, char* salt, char* hash);

pthread_mutex_t save_account_mutex;// = PTHREAD_MUTEX_INITIALIZER;

#endif