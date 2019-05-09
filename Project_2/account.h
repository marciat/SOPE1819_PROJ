//Header file that declares functions used with accounts
#ifndef _ACCOUNT_H
#define _ACCOUNT_H

#include "parse.h"

bank_account_t* create_client_account(client_inf* client_information);

void inquiry_balance(uint32_t account_id, char* password);

void money_transfer(uint32_t account_id, char* password, uint32_t new_account_id, uint32_t balance);

typedef struct bank_account {
  uint32_t account_id;
  char hash[HASH_LEN + 1];
  char salt[SALT_LEN + 1];
  uint32_t balance;
} bank_account_t;

void salt_generator(char* salt);

#endif