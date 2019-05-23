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
#include "sope.h"

//Header files created by us
#include "server.h"
#include "account.h"
#include "reqqueue.h"

pthread_t* threads;
sem_t empty, full;
bool server_run;
pthread_mutex_t server_run_mutex;
pthread_mutex_t srv_mutex;
pthread_mutex_t request_queue_mutex;
int write_fifo, srv_fifo;

unsigned num_run_threads;
pthread_mutex_t run_threads_mutex;

Queue* request_queue;

void* bank_office(void* index){
	int thread_index = *(int*)index;
	int sem_t_value = 0;

	while(true){
		tlv_request_t* request = malloc(MAX_PASSWORD_LEN*2 + 30);
		if(logSyncMech(server_logfile, thread_index, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_CONSUMER, pthread_self()) < 0){
			printf("Log sync mech error!\n");
		}
		if(pthread_mutex_lock(&server_run_mutex) < 0){
			perror("pthread_mutex_lock");
			pthread_exit((void*)-1);
		}
		if(!server_run && isEmpty(request_queue)){
			if(logSyncMech(server_logfile, thread_index, SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_CONSUMER, pthread_self()) < 0){
				printf("Log sync mech error!\n");
			}
			if(pthread_mutex_unlock(&server_run_mutex) < 0){
				perror("pthread_mutex_unlock");
				pthread_exit((void*)-1);
			}
			free(request);
			break;
		}
		if(logSyncMech(server_logfile, thread_index, SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_CONSUMER, pthread_self()) < 0){
			printf("Log sync mech error!\n");
		}
		if(pthread_mutex_unlock(&server_run_mutex) < 0){
			perror("pthread_mutex_unlock");
			pthread_exit((void*)-1);
		}

		if(sem_getvalue(&full, &sem_t_value) < 0){ 
			perror("sem_getvalue");
			pthread_exit((void*)-1);
		}
		if(logSyncMechSem(server_logfile, thread_index, SYNC_OP_SEM_WAIT, SYNC_ROLE_CONSUMER, pthread_self(), sem_t_value) < 0){
			printf("Log sync mech sem error!\n");
		}

		if(sem_wait(&full)){
			perror("sem_wait");
			pthread_exit((void*)-1);
		}
		if(pthread_mutex_lock(&server_run_mutex) < 0){
			perror("pthread_mutex_lock");
			pthread_exit((void*)-1);
		}
		if(!server_run && isEmpty(request_queue)){
			if(pthread_mutex_unlock(&server_run_mutex) < 0){
				perror("pthread_mutex_unlock");
				pthread_exit((void*)-1);
			}
			break;
		}
		if(pthread_mutex_unlock(&server_run_mutex) < 0){
			perror("pthread_mutex_unlock");
			pthread_exit((void*)-1);
		}
		
		if(logSyncMech(server_logfile, thread_index, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_CONSUMER, pthread_self()) < 0){
			printf("Log sync mech error!\n");
		}
		if(pthread_mutex_lock(&request_queue_mutex) < 0){
			perror("pthread_mutex_lock");
			pthread_exit((void*)-1);
		}

		*request = Dequeue(request_queue)->info;

		if(logSyncMech(server_logfile, thread_index, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_CONSUMER, pthread_self()) < 0){
			printf("Log sync mech error!\n");
		}
		if(logRequest(server_logfile, thread_index, request) < 0){
			printf("Log request error!\n");
		}
		if(pthread_mutex_lock(&run_threads_mutex) < 0){
			perror("pthread_mutex_lock");
			pthread_exit((void*)-1);
		}
		num_run_threads++;
		if(logSyncMech(server_logfile, thread_index, SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_CONSUMER, pthread_self()) < 0){
			printf("Log sync mech error!\n");
		}
		if(pthread_mutex_unlock(&run_threads_mutex) < 0){
			perror("pthread_mutex_unlock");
			pthread_exit((void*)-1);
		}

		if(logSyncMech(server_logfile, thread_index, SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_CONSUMER, pthread_self()) < 0){
			printf("Log sync mech error!\n");
		}
		if(pthread_mutex_unlock(&request_queue_mutex) < 0){
			perror("pthread_mutex_unlock");
			pthread_exit((void*)-1);
		}

		if(sem_post(&empty)){
			perror("sem_post");
			pthread_exit((void*)-1);
		}
		if(sem_getvalue(&full, &sem_t_value) < 0){ 
			perror("sem_getvalue");
			pthread_exit((void*)-1);
		}
		if(logSyncMechSem(server_logfile, thread_index, SYNC_OP_SEM_POST, SYNC_ROLE_CONSUMER, request->value.header.pid, sem_t_value) < 0){
			printf("Log sync mech sem error!\n");
		}

		ret_code_t ret_value;
		tlv_reply_t reply;
		switch(request->type){
			case OP_CREATE_ACCOUNT:
				ret_value = create_client_account(&request->value, pthread_self(), request->value.header.op_delay_ms, &reply);
				reply.value.header.ret_code = ret_value;
				send_reply(request, &reply);
				if(logReply(server_logfile, thread_index, &reply) < 0){
					printf("Log reply error!\n");
				}
				break;
			case OP_BALANCE:
				ret_value = check_balance(request->value.header.account_id, request->value.header.password, request->value.header.op_delay_ms, &reply, thread_index);

				reply.value.header.ret_code = ret_value;
				send_reply(request, &reply);
				if(logReply(server_logfile, thread_index, &reply) < 0){
					printf("Log reply error!\n");
				}
				break;
			case OP_TRANSFER:
				ret_value = money_transfer(request->value.header.account_id, request->value.header.password, request->value.transfer.account_id, request->value.transfer.amount, request->value.header.op_delay_ms, &reply, thread_index);

				reply.value.header.ret_code = ret_value;
				send_reply(request, &reply);
				if(logReply(server_logfile, thread_index, &reply) < 0){
					printf("Log reply error!\n");
				}
				break;
			case OP_SHUTDOWN:
				ret_value = shutdown_server(request->value.header.account_id, request->value.header.password, request->value.header.op_delay_ms, &reply, thread_index);
				
				if(ret_value == RC_OK){
					if(pthread_mutex_lock(&server_run_mutex) < 0){
						perror("pthread_mutex_lock");
						pthread_exit((void*)-1);
					}
					server_run = false;
					if(pthread_mutex_unlock(&server_run_mutex) < 0){
						perror("pthread_mutex_unlock");
						pthread_exit((void*)-1);
					}

					if(logSyncMech(server_logfile, thread_index, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_CONSUMER, pthread_self()) < 0){
						printf("Log sync mech error!\n");
					}
					if(pthread_mutex_lock(&run_threads_mutex) < 0){
						perror("pthread_mutex_lock");
						pthread_exit((void*)-1);
					}
					num_run_threads--;
					reply.value.shutdown.active_offices = num_run_threads;
					num_run_threads++;
					if(logSyncMech(server_logfile, thread_index, SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_CONSUMER, pthread_self()) < 0){
						printf("Log sync mech error!\n");
					}
					if(pthread_mutex_unlock(&run_threads_mutex) < 0){
						perror("pthread_mutex_unlock");
						pthread_exit((void*)-1);
					}

					close(write_fifo);
					if(fchmod(srv_fifo, 0444) < 0){
						perror("fchmod");
						pthread_exit((void*)-1);
					}
				}	

				reply.value.header.ret_code = ret_value;
				send_reply(request, &reply);
				if(logReply(server_logfile, thread_index, &reply) < 0){
					printf("Log reply error!\n");
				}	
				break;
			default:
				//error
				break;
		}
		if(logSyncMech(server_logfile, thread_index, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_CONSUMER, pthread_self()) < 0){
			printf("Log sync mech error!\n");
		}
		if(pthread_mutex_lock(&run_threads_mutex) < 0){
			perror("pthread_mutex_lock");
			pthread_exit((void*)-1);
		}
		num_run_threads--;
		if(logSyncMech(server_logfile, thread_index, SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_CONSUMER, pthread_self()) < 0){
			printf("Log sync mech error!\n");
		}
		if(pthread_mutex_unlock(&run_threads_mutex) < 0){
			perror("pthread_mutex_unlock");
			pthread_exit((void*)-1);
		}
		free(request);
	}

	return NULL;
}

int main(int argc, char* argv[]){
	setbuf(stdout, NULL);
	srand(time(NULL));

	server_logfile = open(SERVER_LOGFILE, O_WRONLY | O_APPEND | O_CREAT, 0777); //OPENING SERVER LOGFILE
	if(server_logfile < 0){
		perror("open server logfile");
		exit(-1);
	}

	server_run = true;

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
		exit(-1);
	}

	if(!check_number(argv[1])){
		printf("First argument must be a positive integer.\n");
		exit(-1);
	}

	if(atoi(argv[1]) > MAX_BANK_OFFICES || atoi(argv[1]) < 1 || strlen(argv[1]) > 9){
		printf("Number of bank offices must be between 1 and 99.\n");
		exit(-1);
	}

	int num_bank_offices = atoi(argv[1]);

	create_admin_account(argv[2], 0);

	for(int i = 0; i < MAX_BANK_ACCOUNTS; i++){
		if(pthread_mutex_init(&account_mutex[i], NULL)){
			perror("pthread_mutex_init");
			exit(-1);
		}
	}

	if(pthread_mutex_init(&srv_mutex, NULL)){
		perror("pthread_mutex_init");
		exit(-1);
	}

	if(pthread_mutex_init(&server_run_mutex, NULL)){
		perror("pthread_mutex_init");
		exit(-1);
	}

	if(pthread_mutex_init(&run_threads_mutex, NULL)){
		perror("pthread_mutex_init");
		exit(-1);
	}

	if(logSyncMechSem(server_logfile, 0, SYNC_OP_SEM_INIT, SYNC_ROLE_PRODUCER, 0, num_bank_offices) < 0){
		printf("Log sync mech sem error!\n");
	}
	if(sem_init(&empty, 0, num_bank_offices) < 0){
		perror("sem_init");
		exit(-1);
	}

	if(logSyncMechSem(server_logfile, 0, SYNC_OP_SEM_INIT, SYNC_ROLE_PRODUCER, 0, 0) < 0){
		printf("Log sync mech sem error!\n");
	}
	if(sem_init(&full, 0, 0) < 0){
		perror("sem_init");
		exit(-1);
	}

	num_run_threads = 0;
	
	threads = malloc(sizeof(pthread_t)*num_bank_offices); 

	int* thread_index = malloc(sizeof(int)*num_bank_offices);

	for(int i = 1; i <= num_bank_offices; i++){
		pthread_t tid;
		thread_index[i-1] = i; 

		if(pthread_create(&tid, NULL, bank_office, &thread_index[i-1])){
			perror("pthread_create");
			exit(-1);
		}
		if(logBankOfficeOpen(server_logfile, i, tid) < 0){
			printf("Log bank office open error!\n");
		}
		threads[i-1] = tid;
	}

	if(mkfifo(SERVER_FIFO_PATH, 0666)){
		perror("mkfifo");
		exit(-1);
	}

	srv_fifo = open(SERVER_FIFO_PATH, O_RDONLY);
	if(srv_fifo < 0){
		perror("open server fifo");
		exit(-1);
	}
	
	write_fifo = open(SERVER_FIFO_PATH, O_WRONLY);
	if(write_fifo < 0){
		perror("open write fifo");
		exit(-1);
	}

	if(pthread_mutex_init(&request_queue_mutex, NULL)){
		perror("pthread_mutex_init");
		exit(-1);
	}
	
	request_queue = ConstructQueue(num_bank_offices);

	int sem_t_value = 0;

	while(true){
		
		tlv_request_t* request = malloc(MAX_PASSWORD_LEN*2 + 30);
		int read_val;
		read_val = read_srv_fifo(srv_fifo, request);
		if(read_val < 0){
			exit(-1);
		}
		if(pthread_mutex_lock(&server_run_mutex) < 0){
			perror("pthread_mutex_lock");
			exit(-1);
		}
		if(!server_run && read_val == 0){
			free(request);
			if(pthread_mutex_unlock(&server_run_mutex) < 0){
				perror("pthread_mutex_unlock");
				exit(-1);
			}
			break;
		}
		if(pthread_mutex_unlock(&server_run_mutex) < 0){
			perror("pthread_mutex_unlock");
			exit(-1);
		}
		if(logRequest(server_logfile, 0, request) < 0){
			printf("Log request error!\n");
		}

		if(sem_getvalue(&empty, &sem_t_value) < 0){
			perror("sem_getvalue");
			exit(-1);
		}
		if(logSyncMechSem(server_logfile, 0, SYNC_OP_SEM_WAIT, SYNC_ROLE_PRODUCER, 0, 0) < 0){
			printf("Log sync mech sem error!\n");
		}
	
		if(sem_wait(&empty)){
			perror("sem_wait");
			exit(-1);
		}

		if(logSyncMech(server_logfile, 0, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_PRODUCER, 0) < 0){
			printf("Log sync mech error!\n");
		}
		if(pthread_mutex_lock(&request_queue_mutex) < 0){
			perror("pthread_mutex_lock");
			exit(-1);
		}

		Enqueue(request_queue, request);

		if(logSyncMech(server_logfile, 0, SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_PRODUCER, 0) < 0){
			printf("Log sync mech error!\n");
		}
		if(pthread_mutex_unlock(&request_queue_mutex) < 0){
			perror("pthread_mutex_unlock");
			exit(-1);
		}
		
		if(sem_post(&full)){
			perror("sem_post");
			exit(-1);
		}
		if(sem_getvalue(&full, &sem_t_value) < 0){
			perror("sem_getvalue");
			exit(-1);
		}
		if(logSyncMechSem(server_logfile, 0, SYNC_OP_SEM_POST, SYNC_ROLE_PRODUCER, request->value.header.pid, sem_t_value) < 0){
			printf("Log sync mech sem error!\n");
		}
		if(pthread_mutex_lock(&server_run_mutex) < 0){
			perror("pthread_mutex_lock");
			exit(-1);
		}

		free(request);
		if(!server_run){
			if(pthread_mutex_unlock(&server_run_mutex) < 0){
				perror("pthread_mutex_unlock");
				exit(-1);
			}
			break;
		}
		if(pthread_mutex_unlock(&server_run_mutex) < 0){
			perror("pthread_mutex_unlock");
			exit(-1);
		}
	}
	
	for(int i = 0; i < num_bank_offices; i++){
		if(sem_post(&full) < 0){
			perror("sem_post");
			exit(-1);
		}
	}

	for(int i = 0; i < num_bank_offices; i++){ //Joining all threads before exiting
	
		if(pthread_join(threads[i], NULL)){
			perror("pthread_join");
			exit(-1);
		}
		if(logBankOfficeClose(server_logfile, 0, i+1) < 0){
			printf("Log bank office close error!\n");
		}
	}

	if(close(srv_fifo)){
		perror("close server fifo");
	}

	free(thread_index);
	free(threads);
	DestructQueue(request_queue);

	if(unlink(SERVER_FIFO_PATH)){
		perror("unlink");
		exit(-1);
	}

	if(close(server_logfile) < 0){
		perror("close server logfile");
		exit(-1);
	}

	for(int i = 0; i < MAX_BANK_ACCOUNTS; i++){
		if(pthread_mutex_destroy(&account_mutex[i])){
			perror("pthread_mutex_destroy");
			exit(-1);
		}
	}
	
	pthread_mutex_unlock(&srv_mutex);

	if(pthread_mutex_destroy(&srv_mutex)){
			perror("pthread_mutex_destroy");
			exit(-1);
	}

	if(pthread_mutex_destroy(&server_run_mutex)){
			perror("pthread_mutex_destroy");
			exit(-1);
	}

	if(pthread_mutex_destroy(&run_threads_mutex)){
			perror("pthread_mutex_destroy");
			exit(-1);
	}

	if(pthread_mutex_destroy(&request_queue_mutex)){
			perror("pthread_mutex_destroy");
			exit(-1);
	}

	if(sem_destroy(&empty)){
			perror("sem_destroy");
			exit(-1);
	}

	if(sem_destroy(&full)){
			perror("sem_destroy");
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
	printf("	Must be between 1 and 99\n\n");

	printf("admin_password:\n");
	printf("	This argument represents the administrator password. It has to be between %d and %d characters long.\n", MIN_PASSWORD_LEN, MAX_PASSWORD_LEN);
}

int read_srv_fifo(int srv_fifo, tlv_request_t* request){
	if(logSyncMech(server_logfile, 0, SYNC_OP_MUTEX_LOCK, SYNC_ROLE_PRODUCER, 0) < 0){
		printf("Log sync mech error!\n");
	}
	if(pthread_mutex_lock(&srv_mutex)){
		perror("pthread_mutex_lock");
	}

	int read_value;
	int read_size = 0;
	uint32_t op_type;
	uint32_t length;

	if((read_value = read(srv_fifo, &op_type, sizeof(int))) <= 0)
		return read_value;
	
	if((read_value = read(srv_fifo, &length, sizeof(uint32_t))) <= 0)
		return read_value;

	read_size+= length;
	req_value_t value;

	if((read_value = read(srv_fifo, &value, read_size)) <= 0)
		return read_value;
	

	request->type = op_type;
	request->length = length;
	request->value = value;

	if(logSyncMech(server_logfile, 0, SYNC_OP_MUTEX_UNLOCK, SYNC_ROLE_PRODUCER, 0) < 0){
		printf("Log sync mech error!\n");
	}
	if(pthread_mutex_unlock(&srv_mutex)){
		perror("pthread_mutex_unlock");
	}

	return 0;
}

void send_reply(tlv_request_t* request, tlv_reply_t* reply){
	char pid[6];
	if (request->value.header.pid < 10000)
	{
		sprintf(pid, "0%d", request->value.header.pid);
	}
	else
	{
		sprintf(pid, "%d", request->value.header.pid);
	}
	char fifo_name[USER_FIFO_PATH_LEN];
	strcpy(fifo_name, USER_FIFO_PATH_PREFIX);
	strcat(fifo_name, pid);

	int usr_fifo = open(fifo_name, O_WRONLY | O_APPEND); //Opening server FIFO for writing
	if (usr_fifo < 0)
	{
		reply->value.header.ret_code = RC_USR_DOWN;
	}else{

		if(write(usr_fifo, reply, sizeof(int)+sizeof(uint32_t)+reply->length) < 0){
			perror("write to user fifo");
			exit(-1);
		}
	}
}