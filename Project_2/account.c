#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "sope.h"

#include "account.h"
#include "constants.h"
#include "types.h"

bank_account_t accounts[MAX_BANK_ACCOUNTS] = { {.account_id = 0} };

void salt_generator(char* salt){
	char string[] = "abcdef0123456789";

	for(int i = 0; i < SALT_LEN; i++){
		salt[i] = string[rand() % (strlen(string))];
	}

	salt[SALT_LEN] = '\0';
}

ret_code_t create_client_account(req_value_t* client_information, int thread_id, uint32_t delay, tlv_reply_t *reply){
	
	reply->type = OP_CREATE_ACCOUNT;
	reply->length = sizeof(rep_header_t);
	reply->value.header.account_id = client_information->header.account_id;
	
	if(client_information->header.account_id != 0){
		return RC_OP_NALLOW;
	}

	if(logSyncMechSem(server_logfile, pthread_self(), SYNC_OP_MUTEX_LOCK, SYNC_ROLE_CONSUMER, 0, 0) < 0){
		printf("Log sync mech sum error!\n");
	}

	if(pthread_mutex_lock(&account_mutex)){
		perror("pthread_mutex_lock");
		exit(-1);
	}
	usleep(delay*1000);

	if(client_information->create.account_id == 0 || accounts[client_information->create.account_id].account_id != 0){
		if(logSyncMechSem(server_logfile, pthread_self(), SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_CONSUMER, 0, 0) < 0){
			printf("Log sync mech sum error!\n");
		}
		if(pthread_mutex_unlock(&account_mutex)){
			perror("pthread_mutex_unlock");
			exit(-1);
		}
		return RC_ID_IN_USE;
	}

	bank_account_t account;

	account.account_id = client_information->create.account_id;
	salt_generator(account.salt);
	get_hash(client_information->create.password, account.salt, account.hash);

	account.balance = client_information->create.balance;
	printf("Balance: %d\n", client_information->create.balance);

	accounts[account.account_id] = account;

	if(logSyncMechSem(server_logfile, pthread_self(), SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_CONSUMER, 0, 0) < 0){
		printf("Log sync mech sum error!\n");
	}
	if(pthread_mutex_unlock(&account_mutex)){
		perror("pthread_mutex_unlock");
		exit(-1);
	}

	if(logAccountCreation(server_logfile, thread_id, &account) < 0){
		printf("Log account creation error!\n");
	}

	return RC_OK;
}

ret_code_t create_admin_account(char* admin_password, int thread_id){
	bank_account_t account;

	account.account_id = ADMIN_ACCOUNT_ID;
	salt_generator(account.salt);

	get_hash(admin_password, account.salt, account.hash);

	account.balance = 0;

	accounts[0] = account;

	if(logAccountCreation(server_logfile, thread_id, &account) < 0){
		printf("Log account creation error!\n");
	}

	return RC_OK; 
}


ret_code_t money_transfer(uint32_t account_id, char* password, uint32_t new_account_id, uint32_t amount, uint32_t delay, tlv_reply_t *reply){
	
	reply->type = OP_TRANSFER;
	reply->length = sizeof(rep_header_t) + sizeof(rep_transfer_t);
	reply->value.header.account_id = account_id;

	if(logSyncMechSem(server_logfile, pthread_self(), SYNC_OP_MUTEX_LOCK, SYNC_ROLE_CONSUMER, 0, 0) < 0){
		printf("Log sync mech sum error!\n");
	}
	if(pthread_mutex_lock(&account_mutex)){
		perror("pthread_mutex_lock");
		exit(-1);
	}
	usleep(delay*1000);

	if(account_id == 0 || new_account_id == 0){
		if(logSyncMechSem(server_logfile, pthread_self(), SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_CONSUMER, 0, 0) < 0){
			printf("Log sync mech sum error!\n");
		}
		if(pthread_mutex_unlock(&account_mutex)){
			perror("pthread_mutex_unlock");
			exit(-1);
		}
		reply->value.transfer.balance = 0;
		return RC_OP_NALLOW;
	}

	if(accounts[account_id].account_id == 0 || accounts[new_account_id].account_id == 0){
		if(logSyncMechSem(server_logfile, pthread_self(), SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_CONSUMER, 0, 0) < 0){
			printf("Log sync mech sum error!\n");
		}
		if(pthread_mutex_unlock(&account_mutex)){
			perror("pthread_mutex_unlock");
			exit(-1);
		}
		reply->value.transfer.balance = 0;
		return RC_ID_NOT_FOUND;
	}

	if(account_id == new_account_id){
		if(logSyncMechSem(server_logfile, pthread_self(), SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_CONSUMER, 0, 0) < 0){
			printf("Log sync mech sum error!\n");
		}
		if(pthread_mutex_unlock(&account_mutex)){
			perror("pthread_mutex_unlock");
			exit(-1);
		}
		reply->value.transfer.balance = accounts[account_id].balance;
		return RC_SAME_ID;
	}

	char new_hash[HASH_LEN];
	get_hash(password, accounts[account_id].salt, new_hash);

	if(strcmp(new_hash, accounts[account_id].hash) != 0){
		if(logSyncMechSem(server_logfile, pthread_self(), SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_CONSUMER, 0, 0) < 0){
			printf("Log sync mech sum error!\n");
		}
		if(pthread_mutex_unlock(&account_mutex)){
			perror("pthread_mutex_unlock");
			exit(-1);
		}
		reply->value.transfer.balance = 0;
		return RC_LOGIN_FAIL;
	}

	if(accounts[account_id].balance - amount < MIN_BALANCE){
		if(logSyncMechSem(server_logfile, pthread_self(), SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_CONSUMER, 0, 0) < 0){
			printf("Log sync mech sum error!\n");
		}
		if(pthread_mutex_unlock(&account_mutex)){
			perror("pthread_mutex_unlock");
			exit(-1);
		}
		reply->value.transfer.balance = accounts[account_id].balance;
		return RC_NO_FUNDS;
	}

	if(accounts[new_account_id].balance + amount > MAX_BALANCE){
		if(logSyncMechSem(server_logfile, pthread_self(), SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_CONSUMER, 0, 0) < 0){
			printf("Log sync mech sum error!\n");
		}
		if(pthread_mutex_unlock(&account_mutex)){
			perror("pthread_mutex_unlock");
			exit(-1);
		}
		reply->value.transfer.balance = accounts[account_id].balance;
		return RC_TOO_HIGH;	
	}

	accounts[account_id].balance -= amount;
	accounts[new_account_id].balance += amount;

	if(logSyncMechSem(server_logfile, pthread_self(), SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_CONSUMER, 0, 0) < 0){
		printf("Log sync mech sum error!\n");
	}
	if(pthread_mutex_unlock(&account_mutex)){
		perror("pthread_mutex_unlock");
		exit(-1);
	}

	reply->value.transfer.balance = accounts[account_id].balance;

	return RC_OK;
}


ret_code_t check_balance(uint32_t account_id, char* password, uint32_t delay, tlv_reply_t *reply){

	reply->type = OP_BALANCE;
	reply->length = sizeof(rep_header_t) + sizeof(rep_balance_t);
	reply->value.header.account_id = account_id;

	if(logSyncMechSem(server_logfile, pthread_self(), SYNC_OP_MUTEX_LOCK, SYNC_ROLE_CONSUMER, 0, 0) < 0){
		printf("Log sync mech sum error!\n");
	}

	if(pthread_mutex_lock(&account_mutex)){
		perror("pthread_mutex_lock");
		exit(-1);
	}
	usleep(delay*1000);

	if(account_id == 0){
		if(logSyncMechSem(server_logfile, pthread_self(), SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_CONSUMER, 0, 0) < 0){
			printf("Log sync mech sum error!\n");
		}
		if(pthread_mutex_unlock(&account_mutex)){
			perror("pthread_mutex_unlock");
			exit(-1);
		}
		reply->value.balance.balance = 0;
		return RC_OP_NALLOW;
	}

	if(accounts[account_id].account_id == 0){
		if(logSyncMechSem(server_logfile, pthread_self(), SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_CONSUMER, 0, 0) < 0){
			printf("Log sync mech sum error!\n");
		}
		if(pthread_mutex_unlock(&account_mutex)){
			perror("pthread_mutex_unlock");
			exit(-1);
		}
		reply->value.balance.balance = 0;
		return RC_ID_NOT_FOUND;	
	}
	char new_hash[HASH_LEN];
	get_hash(password, accounts[account_id].salt, new_hash);

	if(strcmp(new_hash, accounts[account_id].hash) != 0){
		if(logSyncMechSem(server_logfile, pthread_self(), SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_CONSUMER, 0, 0) < 0){
			printf("Log sync mech sum error!\n");
		}
		if(pthread_mutex_unlock(&account_mutex)){
			perror("pthread_mutex_unlock");
			exit(-1);
		}
		reply->value.balance.balance = 0;
		return RC_LOGIN_FAIL;
	}

	if(logSyncMechSem(server_logfile, pthread_self(), SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_CONSUMER, 0, 0) < 0){
		printf("Log sync mech sum error!\n");
	}
	if(pthread_mutex_unlock(&account_mutex)){
		perror("pthread_mutex_unlock");
		exit(-1);
	}

	reply->value.balance.balance = accounts[account_id].balance;

	return RC_OK;
}

void get_hash(char* password, char* salt, char* hash){
	int pipe_fd[2], fork_value;

	if(pipe(pipe_fd)){
		perror("pipe");
		exit(-1);
	}

	if((fork_value = fork()) == -1){
		perror("fork");
		exit(-1);
	}

	else if(fork_value == 0){
		if(dup2(pipe_fd[WRITE], STDOUT_FILENO) == -1){
			perror("dup2");
			exit(-1);
		}

		int new_fd[2];
		if(pipe(new_fd)){
			perror("pipe");
			exit(-1);
		} 
		
		if((fork_value = fork()) == -1){
			perror("fork");
			exit(-1);
		}
		else if(fork_value == 0){

			close(new_fd[READ]);

			if(dup2(new_fd[WRITE], STDOUT_FILENO) == -1){
				perror("dup2");
				exit(-1);
			}

			execlp("echo", "echo", strcat(password, salt), NULL);
			printf("ERROR!!!\n");
			close(new_fd[WRITE]);
			exit(-1);
		}

			close(new_fd[WRITE]);
			waitpid(fork_value, NULL, 0);
				
			dup2(new_fd[READ], STDIN_FILENO);

			execlp("sha256sum", "sha256sum", NULL);
			printf("ERROR!!!\n");
		    close(pipe_fd[WRITE]);
		    exit(-1);
	}
	else{
		close(pipe_fd[WRITE]);
		waitpid(fork_value, NULL, 0);
		while(read(pipe_fd[READ], hash, HASH_LEN) == 0);
		close(pipe_fd[READ]);
	}

	hash[HASH_LEN] = '\0';
}
