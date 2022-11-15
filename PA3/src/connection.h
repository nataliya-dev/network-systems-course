#ifndef PROXY_CONNECTION_H
#define PROXY_CONNECTION_H

#include "parser.h"
#include "utils.h"

int open_listen_socket(int portno);
void *proxy_thread(void *vargp);
#endif