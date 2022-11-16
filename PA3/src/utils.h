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
#include <time.h>
#include <unistd.h>

#define MAXBUF 8192
#define LISTENBUF 1024
#define TIMEOUT 5
#define MAXNAME 150
#define MAXIP 16

typedef struct file_list_s {
  char uri[MAXNAME];
  time_t post_time;
} file_list_t;

pthread_mutex_t fl_lock;
file_list_t file_list[MAXNAME];

typedef struct host_name_s {
  char hostname[MAXNAME];
  char ip[MAXIP];
} host_name_t;

pthread_mutex_t hn_lock;
host_name_t host_list[MAXNAME];

int is_cmd_arg_valid(int argc, char **argv);
char *duplicate_str(const char *str);
char *replace_char(char *str, char find, char replace);
char *encode_file_name(char *str);
char *decode_file_name(char *str);

#endif