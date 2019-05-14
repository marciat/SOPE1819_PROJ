//Header file that declares functions used with accounts
#ifndef _ACCOUNT_H
#define _ACCOUNT_H

#include <pthread.h>

#include "parse.h"
#include "types.h"

int create_client_account(req_value_t* client_information);

int create_admin_account(char* admin_password);

int check_balance(uint32_t account_id, char* password);

int money_transfer(uint32_t account_id, char* password, uint32_t new_account_id, uint32_t balance);

void salt_generator(char* salt);

void get_hash(char* password, char* salt, char* hash);

pthread_mutex_t account_mutex;// = PTHREAD_MUTEX_INITIALIZER;

#endif