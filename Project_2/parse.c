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
	//inf->salt = malloc(SALT_LEN+1);
	size_t password_length = sizeof(argv[2])*sizeof(char);
	inf->admin_password = malloc(password_length);
	inf->admin_password = memcpy(inf->admin_password, argv[2], password_length);

	//salt_generator(inf->salt);

	/*char* password = malloc(sizeof(argv[2]));

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
	}*/
}

void parse_client_inf(char* argv[], client_inf* inf){
	inf->account_password = malloc(MAX_PASSWORD_LEN+1);
	inf->operation_arguments = malloc(100);

	inf->account_id = atoi(argv[1]);
	inf->account_password = argv[2];
	inf->operation_delay = atoi(argv[3]);
	inf->operation = atoi(argv[4]);
	inf->operation_arguments = argv[5];

}

void free_server_information(server_inf* server_information){
	free(server_information->admin_password);
	//free(server_information->salt);
	free(server_information);
}

void free_client_information(client_inf* client_information){
	free(client_information->account_password);
	free(client_information->operation_arguments);
	free(client_information);
}

int check_number(char* number_bank_offices){

	for(int i = 0; i < (int)strlen(number_bank_offices); i++){
		if(!isdigit(number_bank_offices[i])){
			return 0;
		}
	}
	return 1;
}