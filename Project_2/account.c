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

void salt_generator(char* salt){
	char string[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

	for(int i = 0; i < SALT_LEN; i++){
		salt[i] = string[rand() % (strlen(string))];
	}

	salt[SALT_LEN] = '\0';
}

bank_account_t* create_client_account(client_inf* client_information){
	usleep(client_information->operation_delay); //TODO Probably change this
	bank_account_t* account = (bank_account_t*)malloc(sizeof(bank_account_t));

	account->account_id = client_information->account_id;
	salt_generator(account->salt);

	//Creating hash
	char* password = malloc(strlen(client_information->account_password)+strlen(account->salt));
	strcpy(password, client_information->account_password);
	int fd[2], fork_value;

	if(pipe(fd)){
		perror("pipe");
		exit(-1);
	}

	if((fork_value = fork()) == -1){
		perror("fork");
		exit(-1);
	}
	else if(fork_value == 0){
		if(dup2(fd[WRITE], STDOUT_FILENO) == -1){
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
			execlp("echo", "echo", strcat(password, account->salt), NULL);
			printf("ERROR!!!\n");
			close(new_fd[WRITE]);
			exit(-1);
		}

		close(new_fd[WRITE]);
		waitpid(fork_value, NULL, 0);
		
		dup2(new_fd[READ], STDIN_FILENO);

		execlp("sha256sum", "sha256sum", NULL);
		printf("ERROR!!!\n");
        close(fd[WRITE]);
        exit(-1);
	}
	else{
		close(fd[WRITE]);
		waitpid(fork_value, NULL, 0);
		while(read(fd[READ], account->hash, HASH_LEN) == 0);
		close(fd[READ]);
	}

	account->balance = 0;

	return account; //Probably this don't works
}

bank_account_t* create_admin_account(char* admin_password){
	bank_account_t* account = (bank_account_t*)malloc(sizeof(bank_account_t));

	account->account_id = ADMIN_ACCOUNT_ID;
	salt_generator(account->salt);

	//Creating hash
	char* password = malloc(strlen(admin_password)+strlen(account->salt));
	strcpy(password, admin_password);
	int fd[2], fork_value;

	if(pipe(fd)){
		perror("pipe");
		exit(-1);
	}

	if((fork_value = fork()) == -1){
		perror("fork");
		exit(-1);
	}
	else if(fork_value == 0){
		if(dup2(fd[WRITE], STDOUT_FILENO) == -1){
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
			execlp("echo", "echo", strcat(password, account->salt), NULL);
			printf("ERROR!!!\n");
			close(new_fd[WRITE]);
			exit(-1);
		}

		close(new_fd[WRITE]);
		waitpid(fork_value, NULL, 0);
		
		dup2(new_fd[READ], STDIN_FILENO);

		execlp("sha256sum", "sha256sum", NULL);
		printf("ERROR!!!\n");
        close(fd[WRITE]);
        exit(-1);
	}
	else{
		close(fd[WRITE]);
		waitpid(fork_value, NULL, 0);
		while(read(fd[READ], account->hash, HASH_LEN) == 0);
		close(fd[READ]);
	}

	account->balance = 0;

	return account; //Probably this don't works
}

void money_transfer(uint32_t account_id, char* password, uint32_t new_account_id, uint32_t balance){
	if(pthread_mutex_lock(&save_account_mutex)){
		perror("pthread_mutex_lock");
		exit(-1);
	}

	int fd = open("accounts.txt", O_RDONLY);
	if(fd < 0){
		perror("open");
		exit(-1);
	}

	char* string = malloc(WIDTH_ID); 
	char* balance_string = malloc(WIDTH_BALANCE);
	bank_account_t tmp_account, account, new_account;

	account.account_id = 0; new_account.account_id = 0; //Initialize variables to check if they exist later

	while(read(fd, string, WIDTH_ID) > 0){
		tmp_account.account_id = atoi(string);
		read(fd, tmp_account.hash, HASH_LEN+1);
		read(fd, tmp_account.salt, HASH_LEN+1);
		memset(string, '\0', WIDTH_ID);
		read(fd, balance_string, WIDTH_BALANCE+1);
		tmp_account.balance = atoi(balance_string);
		memset(balance_string, '\0', WIDTH_BALANCE+1);

		if(tmp_account.account_id == account_id){
			tmp_account.hash[HASH_LEN] = '\0';
			tmp_account.salt[HASH_LEN] = '\0';
			
			//Creating hash
			char* hash = malloc(HASH_LEN+1);

			get_hash(password, tmp_account.salt, hash);
			
			if(strcmp(hash, tmp_account.hash) == 0){ //Found first account
				account = tmp_account;
				if(account.balance < balance){
					free(hash);
					free(string);
					write(STDOUT_FILENO, "OPERATION FAILED: Insufficient Money!!!\n", 40);
					if(pthread_mutex_unlock(&save_account_mutex)){
						perror("pthread_mutex_unlock");
						exit(-1);
					}
					return;
				}
			}
			else{
				free(hash);
				free(string);
				write(STDOUT_FILENO, "OPERATION FAILED: Invalid Password!!!\n", 38);
				if(pthread_mutex_unlock(&save_account_mutex)){
					perror("pthread_mutex_unlock");
					exit(-1);
				}
				return;				
			}

			free(hash);
		}
		else if(tmp_account.account_id == new_account_id){
			new_account = tmp_account;
		}

		if(account.account_id != 0 && new_account.account_id != 0)
			break;
	}

	free(string);
	free(balance_string);

	if(account.account_id == 0 || new_account.account_id == 0){
		write(STDOUT_FILENO, "OPERATION FAILED: Account Does Not Exist!!!\n", 44);
	}

	if(close(fd)){
		perror("close");
		exit(-1);
	}	


	//FIND ACCOUNTS AND TRANSFER THE MONEY

	if(pthread_mutex_unlock(&save_account_mutex)){
		perror("pthread_mutex_unlock");
		exit(-1);
	}
}

uint32_t check_balance(uint32_t account_id, char* password){

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

	int fd = open("accounts.txt", O_RDONLY);
	if(fd < 0){
		perror("open");
		exit(-1);
	}

	char* string = malloc(WIDTH_ID); 
	char* balance_string = malloc(WIDTH_BALANCE);
	bank_account_t tmp_account;

	while(read(fd, string, WIDTH_ID) > 0){
		tmp_account.account_id = atoi(string);
		read(fd, tmp_account.hash, HASH_LEN+1);
		read(fd, tmp_account.salt, HASH_LEN+1);
		memset(string, '\0', WIDTH_ID);
		read(fd, balance_string, WIDTH_BALANCE+1);
		tmp_account.balance = atoi(balance_string);
		memset(balance_string, '\0', WIDTH_BALANCE+1);

		if(tmp_account.account_id == account_id){
			tmp_account.hash[HASH_LEN] = '\0';
			tmp_account.salt[HASH_LEN] = '\0';
			
			//Creating hash
			char* hash = malloc(HASH_LEN+1);
			get_hash(password, tmp_account.salt, hash);

			if(hash == tmp_account.hash){
				free(hash);
				break;
			}else{
				free(hash);
				if(pthread_mutex_unlock(&save_account_mutex)){
					perror("pthread_mutex_unlock");
					exit(-1);
				}
				return RC_OTHER;
			}
		}
	}

	free(string);
	free(balance_string);

	if(close(fd)){
		perror("close");
		exit(-1);
	}	

	if(pthread_mutex_unlock(&save_account_mutex)){
		perror("pthread_mutex_unlock");
		exit(-1);
	}

	return tmp_account.balance;
}

void create_account_storage(){
	int fd = open(ACCOUNT_LIST, O_CREAT, 0777);
	if(fd < 0){
		perror("open");
		exit(-5);
	}
	if(close(fd)){
		perror("close");
		exit(-5);
	}
}

void print_account_to_file(bank_account_t *account){

	int fd = open(ACCOUNT_LIST, O_WRONLY | O_APPEND, 0777);
	if(fd < 0){
		perror("open");
		exit(-5);
	}


	char* account_info = malloc(WIDTH_ID+WIDTH_BALANCE+SALT_LEN+HASH_LEN);
	sprintf(account_info, "%d\n%s\n%s\n%d\n", account->account_id, account->hash, account->salt, account->balance);

	if(pthread_mutex_lock(&save_account_mutex)){
		perror("pthread_mutex_lock");
		exit(-1);
	}

	if(write(fd, account_info, strlen(account_info)) < 0){
		perror("write");
		exit(-1);
	}

	if(pthread_mutex_unlock(&save_account_mutex)){
		perror("pthread_mutex_unlock");
		exit(-1);
	}

	free(account_info);

	close(fd);
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
