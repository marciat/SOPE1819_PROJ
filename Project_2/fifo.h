//Header file that declares variables used with fifos 
#ifndef _FIFO_H
#define _FIFO_H

#include <pthread.h>

pthread_cond_t srv_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t srv_mutex = PTHREAD_MUTEX_INITIALIZER;

#endif