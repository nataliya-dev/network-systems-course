
#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAXLINE 8192 /* max text line length */
#define MAXBUF 8192  /* max I/O buffer size */
#define LISTENQ 1024 /* second argument to listen() */

char command_buf[MAXLINE];

int is_cmd_arg_valid(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <port>\n", argv[0]);
    return -1;
  }
  printf("Port: %s\n", argv[1]);
  return 1;
}

int open_listenfd(int portno) {
  int sockfd, optval = 1;
  struct sockaddr_in servaddr;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1) {
    printf("Socket creation failed...\n");
    exit(EXIT_FAILURE);
  } else {
    printf("Socket successfully created..\n");
  }

  /* Eliminates "Address already in use" error from bind. */
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval,
                 sizeof(int)) < 0) {
    printf("Socket option failed...\n");
    exit(EXIT_FAILURE);
  }

  bzero(&servaddr, sizeof(servaddr));

  // assign IP, PORT
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(portno);

  // Binding newly created socket to given IP and verification
  if ((bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) != 0) {
    printf("Socket bind failed...\n");
    exit(EXIT_FAILURE);
  } else {
    printf("Socket successfully binded..\n");
  }

  /* Make it a listening socket ready to accept connection requests */
  if (listen(sockfd, LISTENQ) < 0) {
    printf("Listening socket failed...\n");
    exit(EXIT_FAILURE);
  }

  return sockfd;
}

void echo(int connfd) {
  size_t n;
  char buf[MAXLINE];
  char httpmsg[] =
      "HTTP/1.1 200 Document "
      "Follows\r\nContent-Type:text/"
      "html\r\nContent-Length:32\r\n\r\n<html><h1>Hello CSCI4273 Course!</h1>";

  n = read(connfd, buf, MAXLINE);
  printf("server received the following request:\n%s\n", buf);
  strcpy(buf, httpmsg);
  printf("server returning a http message with the following content.\n%s\n",
         buf);
  write(connfd, buf, strlen(httpmsg));
}

void read_command(int connfd) {
  int rcvd, fd, bytes_read;
  // char *ptr;

  bzero(command_buf, MAXLINE);
  rcvd = recv(connfd, (char *)command_buf, MAXLINE, 0);

  if (rcvd < 0) {
    printf("recv() error\n");
    exit(EXIT_FAILURE);
  } else if (rcvd == 0) {
    printf("client disconnected\n");
    exit(EXIT_FAILURE);
  } else {
    printf("msg rcvd %s\n", command_buf);
  }

  command_buf[rcvd] = '\0';

  char *method,  // "GET" or "POST"
      *uri,      // "/index.html" things before '?'
      *qs,       // "a=1&b=2"     things after  '?'
      *prot;     // "HTTP/1.1"

  method = strtok(command_buf, " \t\r\n");
  uri = strtok(NULL, " \t");
  prot = strtok(NULL, " \t\r\n");

  printf("method: %s\n", method);
  printf("uri: %s\n", uri);
  printf("prot: %s\n", prot);

  const char ch = '/';
  char *ret;
  ret = strchr(uri, ch);
  if (ret == NULL) {
    printf("No charachter found |%c|\n", ch);
    exit(EXIT_FAILURE);
  } else {
    *ret++;
  }

  printf("String after |%c| is - |%s|\n", ch, ret);
  uri = ret;

  if (access(uri, F_OK) == 0) {
    printf("file exists\n");
  } else {
    printf("so such file\n");
    exit(EXIT_FAILURE);
  }

  char *code = " 200";
  char *info = " Document Follows";
  char header_buf[MAXLINE];
  sprintf(header_buf, "%s%s", prot, code);
  sprintf(header_buf, "%s%s", header_buf, info);

  printf("Header %s\n", header_buf);
}

void *thread(void *vargp) {
  int connfd = *((int *)vargp);
  pthread_detach(pthread_self());
  free(vargp);
  read_command(connfd);
  close(connfd);
  return NULL;
}
int main(int argc, char **argv) {
  if (is_cmd_arg_valid(argc, argv) == -1) {
    exit(EXIT_FAILURE);
  }
  int portno = atoi(argv[1]);

  int listenfd, *connfdp, port, clientlen = sizeof(struct sockaddr_in);
  struct sockaddr_in clientaddr;
  pthread_t tid;

  listenfd = open_listenfd(portno);

  while (1) {
    connfdp = malloc(sizeof(int));
    *connfdp = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);
    pthread_create(&tid, NULL, thread, connfdp);
  }

  return 0;
}
