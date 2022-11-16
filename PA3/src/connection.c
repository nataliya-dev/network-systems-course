#include "connection.h"

int open_listen_socket(int portno) {
  int socket_fd, optval = 1;
  struct sockaddr_in servaddr;

  socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd == -1) {
    printf("Socket creation failed...\n");
    exit(EXIT_FAILURE);
  } else {
    printf("Socket successfully created..\n");
  }

  if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval,
                 sizeof(int)) < 0) {
    printf("Socket address reuse option failed...\n");
    exit(EXIT_FAILURE);
  }

  struct timeval tv;
  tv.tv_usec = 0;
  tv.tv_sec = 100;
  if (setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv,
                 sizeof tv)) {
    printf("Socket timeout option failed...\n");
    exit(EXIT_FAILURE);
  }

  bzero(&servaddr, sizeof(servaddr));

  servaddr.sin_family = AF_INET;  // IPv4
  servaddr.sin_addr.s_addr =
      htonl(INADDR_ANY);  // general purpose, for any available interface
  servaddr.sin_port = htons(portno);  // based on user input

  if ((bind(socket_fd, (struct sockaddr *)&servaddr, sizeof(servaddr))) != 0) {
    printf("Socket bind failed...\n");
    exit(EXIT_FAILURE);
  } else {
    printf("Socket successfully binded..\n");
  }

  if (listen(socket_fd, LISTENBUF) <
      0) {  // passive socket that accepts connections
    printf("Listening socket failed...\n");
    exit(EXIT_FAILURE);
  }

  return socket_fd;
}

void *proxy_thread(void *vargp) {
  int connfd = *((int *)vargp);
  pthread_detach(pthread_self());
  free(vargp);
  exchange_data(connfd);
  close(connfd);
  printf("Closing connection.\n");
  return NULL;
}
