#ifndef PROXY_UTILS_H
#define PROXY_UTILS_H

#include <arpa/inet.h>
#include <ctype.h>
#include <fcntl.h>
#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAXBUF 81920
#define LISTENBUF 1024

int is_cmd_arg_valid(int argc, char **argv);
char *duplicate_str(const char *str);
#endif