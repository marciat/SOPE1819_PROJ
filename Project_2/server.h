// Header file that declares every function and data structure used in the main function of server
#ifndef _SERVER_H
#define _SERVER_H

void server_help();

int read_srv_fifo(int srv_fifo, tlv_request_t* request);

void send_reply(tlv_request_t* request, tlv_reply_t* reply);

#endif