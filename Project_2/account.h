//Header file that declares functions used with accounts
#ifndef _ACCOUNT_H
#define _ACCOUNT_H

#include <pthread.h>

#include "parse.h"
#include "types.h"

ret_code_t create_client_account(req_value_t* client_information, int thread_id, uint32_t delay, tlv_reply_t *reply);

ret_code_t create_admin_account(char* admin_password, int thread_id);

ret_code_t check_balance(uint32_t account_id, char* password, uint32_t delay, tlv_reply_t *reply, int thread_id);

ret_code_t money_transfer(uint32_t account_id, char* password, uint32_t new_account_id, uint32_t amount, uint32_t delay, tlv_reply_t *reply, int thread_id);

ret_code_t shutdown_server(uint32_t account_id, char* password, uint32_t delay, tlv_reply_t *reply, int thread_id);

void salt_generator(char* salt);

void get_hash(char* password, char* salt, char* hash);

pthread_mutex_t account_mutex[MAX_BANK_ACCOUNTS];

int server_logfile;

#endif