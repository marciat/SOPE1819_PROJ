#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <wait.h>

#include "parse.h"
#include "constants.h"

void parse_server_inf(char* argv[], server_inf* inf){
	inf->num_bank_offices = atoi(argv[1]);
	inf->salt = malloc(SALT_LEN+1);
	inf->admin_password = malloc(HASH_LEN+1);

	salt_generator(inf->salt);

	char* password = malloc(sizeof(argv[2]));

	//rem_quot(password, argv[2]);

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
		password = realloc(password, strlen(password)+strlen(inf->salt)+1);
		if(password == NULL){
			printf("Realloc failed!\n");
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
			execlp("echo", "echo", strcat(password, inf->salt), NULL);
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
		while(read(fd[READ], inf->admin_password, HASH_LEN) == 0);
		close(fd[READ]);
	}
}

void salt_generator(char* salt){
	char string[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

	for(int i = 0; i < SALT_LEN; i++){
		salt[i] = string[rand() % (strlen(string))];
	}

	salt[SALT_LEN] = '\0';
}

void rem_quot(char* password, char* pass_with_quot){
	for(int i = 1; i < (int)strlen(pass_with_quot)-1; i++){
		password[i] = pass_with_quot[i];
	}
}

void free_server_information(server_inf* server_information){
	free(server_information->admin_password);
	free(server_information->salt);
	free(server_information);
}

void check_number(char* number_bank_offices){

	for(int i = 0; i < (int)strlen(number_bank_offices); i++){
		if(!isdigit(number_bank_offices[i])){
			printf("First argument must be a positive integer.\n");
			exit(-3);
		}
	}
}