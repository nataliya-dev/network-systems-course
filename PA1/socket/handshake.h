

#include <arpa/inet.h>
#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MAXLINE 1024

int handshake(const void *command_buf, int sockfd,
              const struct sockaddr *dest_addrs, socklen_t dest_addrlen,
              struct sockaddr *src_addr, socklen_t *src_addrlen) {
  ssize_t is_hs_sent =
      sendto(sockfd, command_buf, MAXLINE, 0, dest_addrs, dest_addrlen);
  if (is_hs_sent == -1) {
    printf("sendto error\n");
    return -1;
  }

  printf("Waiting for ack.\n");

  int is_hs_ack = 0;
  socklen_t addrlen = sizeof(struct sockaddr_in);
  ssize_t is_hs_recvd =
      recvfrom(sockfd, &is_hs_ack, sizeof(is_hs_ack), 0, src_addr, src_addrlen);
  if (is_hs_recvd == -1) {
    printf("recvfrom error\n");
    return -1;
  } else if (is_hs_ack != 1) {
    printf("Nack\n");
    return -1;
  }
  return 1;
}