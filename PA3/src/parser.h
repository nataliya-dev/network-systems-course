#ifndef PROXY_PARSER_H
#define PROXY_PARSER_H

#include "utils.h"

void exchange_data(int connfd);
int add_file_to_cache(char* uri);
int check_file_in_cache(char* uri);
int is_valid(const char* method, const char* uri, const char* prot);
void send_error(int connfd);
int open_io_socket(const char* hostname, const char* ip, const int port);
int send_cached_file(int connfd, char* uri);
void remove_cached_files();
int find_hostname_in_cache(const char* hostname, char* ip);
void store_hostname_in_cache(const char* hostname, const char* ip);
int is_blacklisted(const char* hostname);
void send_forbidden(int connfd);
#endif