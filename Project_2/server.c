#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include <semaphore.h>
#include <pthread.h>

//Given header files
#include "constants.h"
#include "types.h"

//Header files created by us
#include "server.h"
#include "account.h"
#include "fifo.h"
#include "reqqueue.h"

pthread_t* threads;
//mqd_t order_queue;
//char order_queue_name[] = "order_queue";
sem_t empty, full;
bool server_run;

Queue* request_queue;

void* bank_office(){

	while(server_run){
		tlv_request_t* request = malloc(MAX_PASSWORD_LEN*2 + 30);
		sem_wait(&full);
		*request = Dequeue(request_queue)->info;
		sem_post(&empty);
		ret_code_t ret_value;
		switch(request->type){
			case OP_CREATE_ACCOUNT:
				printf("create\n");
				ret_value = create_client_account(&request->value);
				break;
			case OP_BALANCE:
				printf("balance\n");
				ret_value = check_balance(request->value.header.account_id, request->value.header.password);
				break;
			case OP_TRANSFER:
				printf("transfer\n");
				ret_value = money_transfer(request->value.header.account_id, request->value.header.password, request->value.transfer.account_id, request->value.transfer.amount);
				break;
			case OP_SHUTDOWN:
				printf("shutdown\n");
				server_run = false;
				printf("thread:%d\n", server_run);
				//SHUTDOWN SERVER - Terminar ciclo dos balcões
				//Verificar no server se foi recebida a operação (variável global que indica se pode encerrar) 
				//Recolha de todas as threads
				break;
			default:
				//error
				break;
		}
		free(request);
		//////////TODO TIRAR ISTO O PEDRO DISSE
		(void) ret_value;
	}

	printf("ola td bem\n");

	return NULL;
}

int main(int argc, char* argv[]){
	setbuf(stdout, NULL);
	srand(time(NULL));

	server_run = true;

	if(server_run){
		printf("ola\n");
	}

	if(argc == 2 && strcmp(argv[1], "--help") == 0){
		server_help();
		exit(0);
	}

	if(argc != 3){
		printf("Usage: server number_of_bank_offices \"admin_password\"\n");
		printf("Try server --help for more information\n.");
		exit(-1);
	}

	if(strlen(argv[2]) > MAX_PASSWORD_LEN || strlen(argv[2]) < MIN_PASSWORD_LEN){
		printf("Password length must be between 8 to 20 characters.\n");
		exit(-2);
	}

	if(!check_number(argv[1])){
		printf("First argument must be a positive integer.\n");
		exit(-3);
	}

	if(atoi(argv[1]) > MAX_BANK_OFFICES || atoi(argv[1]) < 1 || strlen(argv[1]) > 9){
		printf("Number of bank offices must be between 1 and 99.\n");
		exit(-4);
	}

	int num_bank_offices = atoi(argv[1]);

	//bank_account_t *admin_account = create_admin_account(argv[2]);
	int admin_return = create_admin_account(argv[2]); //TODO Change return value handler
	printf("%d\n", admin_return); //Delete this

	if(pthread_mutex_init(&account_mutex, NULL)){
		perror("pthread_mutex_init");
		exit(-1);
	}
	if(pthread_mutex_init(&srv_mutex, NULL)){
		perror("pthread_mutex_init");
		exit(-1);
	}

	threads = malloc(sizeof(pthread_t)*num_bank_offices); 

	if(mkfifo(SERVER_FIFO_PATH, 0666)){
		perror("mkfifo");
		exit(-1);
	}

	int srv_fifo = open(SERVER_FIFO_PATH, O_RDONLY);
	if(srv_fifo < 0){
		perror("open server fifo");
		exit(-1);
	}

	request_queue = ConstructQueue(5000);

	for(int i = 1; i <= num_bank_offices; i++){
		pthread_t tid = i;
		if(pthread_create(&tid, NULL, bank_office, NULL)){
			perror("pthread_create");
			exit(-1);
		}
		threads[i-1] = tid;
	}

	sem_init(&empty, 0, num_bank_offices);
	sem_init(&full, 0, 0);

	while(server_run){
		sem_wait(&empty);
		printf("main:%d\n", server_run);
		tlv_request_t* request = malloc(MAX_PASSWORD_LEN*2 + 30);
		printf("main:%d\n", server_run);
		read_srv_fifo(srv_fifo, request);
		printf("main:%d\n", server_run);
		Enqueue(request_queue, request);
		printf("main:%d\n", server_run);
		printf("Size read: %d\n", request->length);
		printf("main:%d\n", server_run);
		sem_post(&full);
		printf("main:%d\n", server_run);
		free(request);
	}

	printf("acabou while\n");

	for(int i = 0; i < num_bank_offices; i++){ //Joining all threads before exiting
		if(pthread_join(threads[i], NULL)){
			perror("pthread_join");
			exit(-1);
		}
	}
	if(close(srv_fifo)){
		perror("close server fifo");
	}


	free(threads);
	DestructQueue(request_queue);

	if(unlink(SERVER_FIFO_PATH)){
		perror("unlink");
		exit(-1);
	}

	return 0;
}

void server_help(){
	printf("Usage: server number_of_bank_offices \"admin_password\"\n");
	printf("Creates a homebanking server in your pc\n");
	printf("Example: server 10 \"bad_password\"\n\n");

	printf("number_of_bank_offices:\n"); 
	printf("	This argument is an integer that specifies the number of bank offices to create in the server.\n");
	printf("	Max number of bank offices: 99\n\n");

	printf("admin_password:\n");
	printf("	This argument represents the administrator password. This have to be between quotation marks.\n");
}

void read_srv_fifo(int srv_fifo, tlv_request_t* request){
	if(pthread_mutex_lock(&srv_mutex)){
		perror("pthread_mutex_lock");
	}

	int read_value;
	int read_size = 0;
	uint32_t op_type;
	uint32_t length;

	while((read_value = read(srv_fifo, &op_type, sizeof(int))) == 0){
		if(read_value < 0){
			perror("read server fifo");
		}
		/*if(pthread_cond_wait(&srv_cond, &srv_mutex)){
			perror("pthread_cond_wait");
		}*/
	}
	printf("%d\n", op_type);
	while((read_value = read(srv_fifo, &length, sizeof(uint32_t))) == 0){
		if(read_value < 0){
			perror("read server fifo");
		}
		/*if(pthread_cond_wait(&srv_cond, &srv_mutex)){
			perror("pthread_cond_wait");
		}*/
	}
	printf("%d\n", length);

	read_size+= length;
	req_value_t value;

	while((read_value = read(srv_fifo, &value, read_size)) == 0){
		if(read_value < 0){
			perror("read server fifo");
		}
		/*if(pthread_cond_wait(&srv_cond, &srv_mutex)){
			perror("pthread_cond_wait");
		}*/
	}

	request->type = op_type;
	request->length = length;
	request->value = value;

	if(pthread_mutex_unlock(&srv_mutex)){
		perror("pthread_mutex_unlock");
	}
}