//Header file that declares functions used with accounts
#ifndef _ACCOUNT_H
#define _ACCOUNT_H

#include <pthread.h>

#include "parse.h"

bank_account_t* create_client_account(client_inf* client_information);

void inquiry_balance(uint32_t account_id, char* password);

void money_transfer(uint32_t account_id, char* password, uint32_t new_account_id, uint32_t balance);

void salt_generator(char* salt);

void save_account(bank_account_t* account);

char* subst_null_by_endline(char* string);

pthread_mutex_t* save_account_mutex;

#endif