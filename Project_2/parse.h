//Header file that declares functions and data structures used to parse data

#ifndef _PARSE_H
#define _PARSE_H

#include "types.h"

void parse_client_inf(char* argv[], tlv_request_t* request);

int check_number(char* number_bank_offices);

#define READ 0
#define WRITE 1

#endif