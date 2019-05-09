#include "account.h"

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
	password = memcpy(password, client_information->account_password);
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
