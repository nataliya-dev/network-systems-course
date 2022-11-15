#ifndef PROXY_PARSER_H
#define PROXY_PARSER_H

#include "utils.h"

void exchange_data(int connfd);

int is_valid(const char* method, const char* uri, const char* prot);
void send_error(int connfd);
int open_io_socket(const char* ip, const int portno);

#endif