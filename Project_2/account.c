#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

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

int create_client_account(req_value_t* client_information){
	if(client_information->header.account_id == 0 || accounts[client_information->header.account_id].account_id != 0){
		return RC_ID_IN_USE;
	}

	bank_account_t account;

	account.account_id = client_information->header.account_id;
	salt_generator(account.salt);
	get_hash(client_information->header.password, account.salt, account.hash);

	account.balance = 0;

	accounts[account.account_id] = account;

	return 0;
}

int create_admin_account(char* admin_password){
	bank_account_t account;

	account.account_id = ADMIN_ACCOUNT_ID;
	salt_generator(account.salt);

	get_hash(admin_password, account.salt, account.hash);

	account.balance = 0;

	accounts[0] = account;

	return 0; 
}

int money_transfer(uint32_t account_id, char* password, uint32_t new_account_id, uint32_t amount){
	if(pthread_mutex_lock(&save_account_mutex)){
		perror("pthread_mutex_lock");
		exit(-1);
	}

	if(account_id == 0 || new_account_id == 0){
		if(pthread_mutex_unlock(&save_account_mutex)){
			perror("pthread_mutex_unlock");
			exit(-1);
		}
		return RC_OP_NALLOW;
	}

	if(accounts[account_id].account_id == 0 || accounts[new_account_id].account_id == 0){
		//write(STDOUT_FILENO, "OPERATION FAILED: Account Does Not Exist!!!\n", 44);
		if(pthread_mutex_unlock(&save_account_mutex)){
			perror("pthread_mutex_unlock");
			exit(-1);
		}
		return RC_ID_NOT_FOUND;
	}

	if(account_id == new_account_id){
		if(pthread_mutex_unlock(&save_account_mutex)){
			perror("pthread_mutex_unlock");
			exit(-1);
		}
		return RC_SAME_ID;
	}

	char new_hash[HASH_LEN];
	get_hash(password, accounts[account_id].hash, new_hash);

	if(strcmp(new_hash, accounts[account_id].hash) != 0){
		//write(STDOUT_FILENO, "OPERATION FAILED: Invalid Password!!!\n", 38);
		if(pthread_mutex_unlock(&save_account_mutex)){
			perror("pthread_mutex_unlock");
			exit(-1);
		}
		return RC_LOGIN_FAIL;
	}

	if(accounts[account_id].balance - amount < MIN_BALANCE){
		//write(STDOUT_FILENO, "OPERATION FAILED: Insufficient Money!!!\n", 40);
		if(pthread_mutex_unlock(&save_account_mutex)){
			perror("pthread_mutex_unlock");
			exit(-1);
		}
		return RC_NO_FUNDS;
	}

	if(accounts[new_account_id].balance + amount > MAX_BALANCE){
		if(pthread_mutex_unlock(&save_account_mutex)){
			perror("pthread_mutex_unlock");
			exit(-1);
		}
		return RC_TOO_HIGH;	
	}

	accounts[account_id].balance -= amount;
	accounts[new_account_id].balance += amount;

	if(pthread_mutex_unlock(&save_account_mutex)){
		perror("pthread_mutex_unlock");
		exit(-1);
	}

	return RC_OK;
}

int check_balance(uint32_t account_id, char* password){

	if(pthread_mutex_lock(&save_account_mutex)){
		perror("pthread_mutex_lock");
		exit(-1);
	}

	if(account_id == 0){
		if(pthread_mutex_unlock(&save_account_mutex)){
			perror("pthread_mutex_unlock");
			exit(-1);
		}
		return RC_OP_NALLOW;
	}

	if(accounts[account_id].account_id == 0){
		if(pthread_mutex_unlock(&save_account_mutex)){
			perror("pthread_mutex_unlock");
			exit(-1);
		}
		return RC_ID_NOT_FOUND;	
	}

	char new_hash[HASH_LEN];
	get_hash(password, accounts[account_id].hash, new_hash);

	if(strcmp(new_hash, accounts[account_id].hash) != 0){
		//write(STDOUT_FILENO, "OPERATION FAILED: Invalid Password!!!\n", 38);
		if(pthread_mutex_unlock(&save_account_mutex)){
			perror("pthread_mutex_unlock");
			exit(-1);
		}
		return RC_LOGIN_FAIL;
	}

	printf("%d\n", accounts[account_id].balance);

	if(pthread_mutex_unlock(&save_account_mutex)){
		perror("pthread_mutex_unlock");
		exit(-1);
	}

	//return tmp_account.balance;
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
}
