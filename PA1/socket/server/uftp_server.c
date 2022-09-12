#include <arpa/inet.h>
#include <ctype.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT 8001
#define MAXLINE 1024

void write_file(int sockfd, const struct sockaddr *src_addr,
                socklen_t addrlen) {
  int n;
  FILE *fp;
  char *filename = "foo1.jpg";
  char rcvbuf[MAXLINE];

  fp = fopen(filename, "wb");
  if (fp == NULL) {
    perror("[-]Error in creating file.");
    exit(1);
  }
  while (1) {
    n = recvfrom(sockfd, (char *)rcvbuf, MAXLINE, 0,
                 (struct sockaddr *)&src_addr, &addrlen);
    if (n <= 0) {
      break;
      return;
    }
    // size_t bytes_read;
    // // Read a buffer sized hunk.
    // while ((bytes_read = fread(rcvbuf, 1, sizeof(rcvbuf), in))) {
    //   // Write the hunk, but only as much as was read.
    //   fwrite(rcvbuf, 1, bytes_read, out);
    // }

    fwrite(rcvbuf, 1, sizeof(rcvbuf), fp);
    bzero(rcvbuf, MAXLINE);
  }
  return;
}

int main() {
  int sockfd;
  char rcvbuf[MAXLINE];
  char sndbuf[MAXLINE];
  struct sockaddr_in servaddr, src_addr;
  socklen_t n;
  int i = 0;

  // Creating socket file descriptor
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }

  memset(&servaddr, 0, sizeof(servaddr));
  memset(&src_addr, 0, sizeof(src_addr));

  // Filling server information
  servaddr.sin_family = AF_INET;  // IPv4
  servaddr.sin_addr.s_addr = INADDR_ANY;
  servaddr.sin_port = htons(PORT);

  // Bind the socket with the server address
  if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  socklen_t addrlen = sizeof(struct sockaddr_in);
  write_file(sockfd, (const struct sockaddr *)&src_addr, addrlen);
  return 0;

  printf("The server is ready to receive\n");
  // socklen_t addrlen = sizeof(struct sockaddr_in);
  n = recvfrom(sockfd, (char *)rcvbuf, MAXLINE, 0, (struct sockaddr *)&src_addr,
               &addrlen);

  char *some_addr;
  some_addr = inet_ntoa(src_addr.sin_addr);
  printf("Msg received from: %s\n", some_addr);

  /* captalize the received string */
  while (rcvbuf[i]) {
    sndbuf[i] = toupper(rcvbuf[i]);
    i++;
  }
  sndbuf[i] = '\0';

  sendto(sockfd, (const char *)sndbuf, strlen(sndbuf), 0,
         (const struct sockaddr *)&src_addr, addrlen);

  printf("%s\n", sndbuf);

  close(sockfd);

  return 0;
}
