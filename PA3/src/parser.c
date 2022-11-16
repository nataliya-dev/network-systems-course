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

  printf("method: %s\n", method);
  printf("uri: %s\n", uri);
  printf("prot: %s\n", prot);

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

int open_io_socket(const char* hostname, const int port) {
  int socket_fd;
  struct hostent* address;
  struct sockaddr_in serveraddr;

  socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd == -1) {
    printf("Socket creation failed...\n");
    return -1;
  } else {
    printf("Socket successfully created..\n");
  }

  struct timeval tv;
  tv.tv_usec = 0;
  tv.tv_sec = 2;
  if (setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv,
                 sizeof tv)) {
    printf("Socket timeout option failed...\n");
    return -1;
  }

  address = gethostbyname(hostname);
  if (address == NULL) {
    printf("gethostbyname failed\n");
    return -1;
  }

  char* ip = inet_ntoa(*((struct in_addr*)address->h_addr_list[0]));
  printf("ip: %s\n", ip);

  bzero((char*)&serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  bcopy((char*)address->h_addr_list[0], (char*)&serveraddr.sin_addr.s_addr,
        address->h_length);
  serveraddr.sin_port = htons(port);

  int status =
      connect(socket_fd, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
  if (status < 0) {
    printf("connect failed\n");
    return -1;
  } else {
    printf("Connected\n");
  }

  return socket_fd;
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

    printf("\n===== New msg recieved\n");
    printf("%s\n", data_buf);

    char* data_buf_cp = duplicate_str(data_buf);
    method = strtok(data_buf_cp, " \t\r\n");
    uri = strtok(NULL, " \t\r\n");
    prot = strtok(NULL, " \t\r\n");

    status = is_valid(method, uri, prot);
    if (status != 1) {
      send_error(connfd);
      break;
    }

    char hostname[300];
    extract_host(uri, hostname, sizeof(hostname));
    printf("host: %s\n", hostname);

    int host_fd = open_io_socket(hostname, 80);
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

    printf("writing the following \n%s\n", request_buf);
    int written = write(host_fd, request_buf, strlen(request_buf));
    printf("written: %d\n", written);
    printf("Request to  host sent\n");

    int recvd = 0;

    do {
      bzero(recv_buf, MAXBUF);
      int recvd = recv(host_fd, recv_buf, MAXBUF, 0);
      if (recvd <= 0) {
        break;
      }

      printf("recvd: %d\n", recvd);
      printf("sizeof(recv_buf): %d\n", recvd);
      // printf("recv_buf\n%s\n", recv_buf);
      size_t bytes_written = write(connfd, recv_buf, recvd);
      printf("bytes_written: %ld\n", bytes_written);
    } while (recvd > 0);
  }
}
