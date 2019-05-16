// Header file that declares every function and data structure used in the main function of user
#ifndef _USER_H
#define _USER_H

#include "parse.h"

void user_help();

void write_srv_fifo(int srv_fifo, tlv_request_t* request);

void read_user_fifo(int usr_fifo, tlv_reply_t *reply);

void* zzz();

#endif