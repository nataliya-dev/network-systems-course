#include "parser.h"

void send_error(int connfd) {
  char data_buf[MAXBUF];

  char* prot = "HTTP/1.1 ";
  char* code = "400 ";
  char* info = "Bad Request\r\n";

  sprintf(data_buf, "%s%s%s", prot, code, info);
  // printf("Error msg sent\n%s\n", data_buf);
  write(connfd, data_buf, strlen(data_buf));
}

int is_valid(const char* method, const char* uri, const char* prot) {
  if (method == NULL || uri == NULL || prot == NULL) {
    printf("Unable to parse message\n");
    return -1;
  }

  if (strcmp(method, "GET") != 0) {
    // printf("Only GET command accepted.\n");
    return -1;
  }

  // printf("method: %s\n", method);
  // printf("uri: %s\n", uri);
  // printf("prot: %s\n", prot);

  return 1;
}

void extract_host(const char* szUrl, char* szHost, const size_t size) {
  const char* p;
  char* q = szHost;
  int n = 0;

  szHost[0] = '\0';
  if ((p = strstr(szUrl, "//")) == NULL) return;

  for (p += 2; *p; p++) {
    if (n >= (int)size) break;
    if (*p == ':' || *p == '/') break;
    *q++ = *p;
    n++;
  }
  *q = '\0';
}

int check_file_in_cache(char* uri) {
  pthread_mutex_lock(&fl_lock);
  int status = -1;

  for (size_t i = 0; i < MAXNAME; i++) {
    file_list_t file = file_list[i];
    // printf("file.uri %s at %ld\n", file.uri, i);
    if (strcmp(file.uri, uri) == 0) {
      printf("File exists\n");
      double time_taken = time(NULL) - file.post_time;
      printf("Time taken %f\n", time_taken);
      if (time_taken > TIMEOUT) {
        status = -1;
      } else {
        status = 1;
      }
      break;
    }
  }

  pthread_mutex_unlock(&fl_lock);
  return status;
}

void remove_cached_files() {
  pthread_mutex_lock(&fl_lock);
  for (size_t i = 0; i < MAXNAME; i++) {
    file_list_t file = file_list[i];
    double time_taken = time(NULL) - file.post_time;
    if (strlen(file.uri) > 0 && time_taken > TIMEOUT) {
      strcpy(file_list[i].uri, "");
      char* file_name = encode_file_name(file.uri);
      printf("file_name %s\n", file_name);
      int is_rem = remove(file_name);
      if (is_rem == 0) {
        printf("File deleted successfully.\n");
      } else {
        perror("Delete file error.\n");
      }
    }
  }
  pthread_mutex_unlock(&fl_lock);
}

int add_file_to_cache(char* uri) {
  pthread_mutex_lock(&fl_lock);
  int status = -1;

  for (size_t i = 0; i < MAXNAME; i++) {
    if (strlen(file_list[i].uri) <= 0) {
      strcpy(file_list[i].uri, uri);
      file_list[i].post_time = time(NULL);
      printf("file.uri %s at %ld\n", file_list[i].uri, i);
      status = 1;
      break;
    }
  }
  pthread_mutex_unlock(&fl_lock);
  return status;
}

int open_io_socket(const char* hostname, const char* ip, const int port) {
  int socket_fd;
  struct sockaddr_in serveraddr;
  bzero((char*)&serveraddr, sizeof(serveraddr));

  socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd == -1) {
    printf("Socket creation failed...\n");
    return -1;
  } else {
    printf("Socket successfully created..\n");
  }

  if (strlen(ip) == 0) {
    printf("gethostbyname\n");
    struct hostent* address;
    address = gethostbyname(hostname);
    if (address == NULL) {
      printf("gethostbyname failed\n");
      return -1;
    }
    bcopy((char*)address->h_addr_list[0], (char*)&serveraddr.sin_addr.s_addr,
          address->h_length);
    char* stored_ip = inet_ntoa(*((struct in_addr*)address->h_addr_list[0]));
    store_hostname_in_cache(hostname, stored_ip);
  } else {
    printf("using cached hostname\n");
    inet_aton(ip, &serveraddr.sin_addr);
  }

  // printf("ip: %s\n", ip);
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_port = htons(port);

  int status =
      connect(socket_fd, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
  if (status < 0) {
    printf("connect failed\n");
    return -1;
  } else {
    printf("Connected\n");
  }

  struct timeval tv;
  tv.tv_usec = 0;
  tv.tv_sec = 1;
  if (setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv,
                 sizeof tv)) {
    printf("Socket timeout option failed...\n");
    return -1;
  }

  return socket_fd;
}

int send_cached_file(int connfd, char* uri) {
  char* file_name = encode_file_name(uri);
  int file_desc = open(file_name, O_RDONLY, 0666);
  if (file_desc == -1) {
    printf("Open file error. Try again.\n");
    return -1;
  }
  char data_buf[MAXBUF];

  while (1) {
    bzero(data_buf, MAXBUF);
    size_t bytes_read = read(file_desc, data_buf, MAXBUF);
    if (bytes_read <= 0) {
      break;
    }
    size_t bytes_written = write(connfd, data_buf, bytes_read);
    printf("bytes_written: %ld\n", bytes_written);
    if (bytes_read <= 0) {
      break;
    }
  }
  printf("Finished sending file\n");
  return 1;
}

int find_hostname_in_cache(const char* hostname, char* ip) {
  pthread_mutex_lock(&hn_lock);
  int status = -1;
  for (size_t i = 0; i < MAXNAME; i++) {
    // printf("Hostname at i: %d, %s: %s\n", i, host_list[i].hostname,
    //        host_list[i].ip);
    if (strcmp(host_list[i].hostname, hostname) == 0) {
      strcpy(ip, host_list[i].ip);
      printf("Hostname found %s: %s\n", hostname, ip);
      status = 1;
      break;
    }
  }
  pthread_mutex_unlock(&hn_lock);
  return status;
}

void store_hostname_in_cache(const char* hostname, const char* ip) {
  pthread_mutex_lock(&hn_lock);
  for (size_t i = 0; i < MAXNAME; i++) {
    if (strlen(host_list[i].hostname) == 0) {
      strcpy(host_list[i].hostname, hostname);
      strcpy(host_list[i].ip, ip);
      printf("Hostname added %s: %s\n", host_list[i].hostname, host_list[i].ip);
      break;
    }
  }
  pthread_mutex_unlock(&hn_lock);
}

void exchange_data(int connfd) {
  char data_buf[MAXBUF];
  char recv_buf[MAXBUF];
  char request_buf[MAXBUF];
  char *method, *uri, *prot;
  int status = 0;

  while (1) {
    bzero(data_buf, MAXBUF);
    bzero(recv_buf, MAXBUF);
    bzero(request_buf, MAXBUF);

    int rec = recv(connfd, data_buf, MAXBUF, 0);
    if (rec <= 0) {
      break;
    }

    // printf("\n===== New msg recieved\n");
    // printf("%s\n", data_buf);

    char* data_buf_cp = duplicate_str(data_buf);
    method = strtok(data_buf_cp, " \t\r\n");
    uri = strtok(NULL, " \t\r\n");
    prot = strtok(NULL, " \t\r\n");

    status = is_valid(method, uri, prot);
    if (status != 1) {
      send_error(connfd);
      break;
    }

    for (size_t i = 0; i < MAXNAME; i++) {
      file_list_t file = file_list[i];
      if (strcmp(file.uri, uri) == 0) {
        printf("File exists\n");
      }
    }

    int exists = check_file_in_cache(uri);
    if (exists == 1 && send_cached_file(connfd, uri)) {
      printf("File sent\n");
      break;
    }

    char hostname[MAXNAME];
    extract_host(uri, hostname, sizeof(hostname));
    printf("host: %s\n", hostname);

    char ip[MAXIP];
    find_hostname_in_cache(hostname, ip);

    int host_fd = open_io_socket(hostname, ip, 80);
    if (host_fd == -1) {
      printf("Socket creation failed...\n");
      send_error(connfd);
      break;
    } else {
      printf("Socket successfully created..\n");
    }

    // sprintf(request_buf, "%s%s%s%s%s%s%s%s", method, " / ", prot, "\r\n",
    //         "Host: ", hostname, "\r\nConnection: close", "\r\n\r\n");

    sprintf(request_buf, "%s%s%s%s%s%s%s%s%s%s", method, " ", uri, " ", prot,
            "\r\n", "Host: ", hostname, "\r\nConnection: keep-alive",
            "\r\n\r\n");

    // printf("writing the following \n%s\n", request_buf);
    int written = write(host_fd, request_buf, strlen(request_buf));
    // printf("written: %d\n", written);
    // printf("Request to  host sent\n");

    char* encoded_fn = encode_file_name(uri);
    int file_desc = open(encoded_fn, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (file_desc == -1) {
      printf("Open file error. Try again.\n");
      break;
    }

    while (1) {
      bzero(recv_buf, MAXBUF);
      int recvd = recv(host_fd, recv_buf, MAXBUF, 0);

      if (recvd <= 0) {
        break;
      }

      printf("uri: %s\n", uri);

      size_t bytes_written = write(connfd, recv_buf, recvd);
      printf("bytes_written: %ld\n", bytes_written);

      if (bytes_written < 0) {
        printf("ERROR bytes_written < 0\n");
        break;
      } else if (bytes_written == 0) {
        printf("ERROR bytes_written == 0\n");
        break;
      }

      ssize_t is_written = write(file_desc, recv_buf, recvd);
      if (is_written == -1) {
        printf("Write error. Try again.\n");
        break;
      }
    }
    remove_cached_files();
    add_file_to_cache(uri);
    printf("File transfer done!\n");
  }
}
