#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MAXLINE 1024

char *strdup(const char *str) {
  char *newstr = (char *)malloc(strlen(str) + 1);
  if (newstr) {
    strcpy(newstr, str);
  }
  return newstr;
}

int send_file(FILE *fp, int sockfd, const struct sockaddr *dest_addr,
              int size) {
  char data[MAXLINE] = {0};
  while (fgets(data, MAXLINE, fp) != NULL) {
    int status = sendto(sockfd, data, sizeof(data), 0, dest_addr, size);
    if (status == -1) {
      perror("Error in sending file");
      return 0;
    }
    bzero(data, MAXLINE);
  }
  return 1;
}

char *VALID_COMMANDS[5] = {"get", "put", "delete", "ls", "exit"};
int get_command_option(const char *str) {
  char *token = strtok(strdup(str), " ");
  int len = sizeof(VALID_COMMANDS) / sizeof(VALID_COMMANDS[0]);
  int i = 0;
  for (i = 0; i < len; ++i) {
    if (strcmp(VALID_COMMANDS[i], token) == 0) {
      return i;
    }
  }
  return -1;
}

char *get_filename(const char *str) {
  char *token = strtok(strdup(str), " ");
  token = strtok(NULL, " ");
  return token;
}

int is_cmd_arg_valid(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <hostname> <port>\n", argv[0]);
    return 0;
  }

  printf("Num arguments: %d\n", argc);
  printf("Hostname: %s\n", argv[1]);
  printf("Port number: %s\n", argv[2]);
  return 1;
}

int main(int argc, char **argv) {
  if (is_cmd_arg_valid(argc, argv) == 0) {
    exit(EXIT_FAILURE);
  }

  char *hostname = argv[1];
  int portno = atoi(argv[2]);

  int sockfd;
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) {
    perror("calling socket");
    exit(EXIT_FAILURE);
  }

  // int status;
  // status = fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) | O_NONBLOCK);
  // if (status == -1) {
  //   perror("calling fcntl");
  //   exit(EXIT_FAILURE);
  // }

  struct sockaddr_in servaddr;
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(portno);
  inet_aton(hostname, &servaddr.sin_addr);

  printf("Type one of the following commands:\n");
  printf("get [filename]\n");
  printf("put [filename]\n");
  printf("delete [filename]\n");
  printf("ls\n");
  printf("exit\n\n");

  char sndbuf[MAXLINE];
  if (fgets(sndbuf, sizeof sndbuf, stdin)) {
    sndbuf[strcspn(sndbuf, "\n")] = '\0';
  }

  int option = get_command_option(sndbuf);
  if (option == -1) {
    printf("%s command is invalid. Try again.\n", sndbuf);
  }

  if (option == 1) {
    // char *filename = get_filename(sndbuf);
    char *filename = "foo1.jpg";
    printf("filename: %s\n", filename);
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) {
      perror("file error");
      exit(EXIT_FAILURE);
    }

    int status = send_file(fp, sockfd, (const struct sockaddr *)&servaddr,
                           sizeof(servaddr));
    if (status == 0) {
      perror("send file error");
      exit(EXIT_FAILURE);
    }
    fclose(fp);
  }

  socklen_t n, len;
  // sendto(sockfd, (const char *)sndbuf, strlen(sndbuf), 0,
  //        (const struct sockaddr *)&servaddr, sizeof(servaddr));

  char rcvbuf[MAXLINE];
  n = recvfrom(sockfd, (char *)rcvbuf, MAXLINE, MSG_WAITALL,
               (struct sockaddr *)&servaddr, &len);
  rcvbuf[n] = '\0';
  printf("%s\n", rcvbuf);

  close(sockfd);
  return 0;
}
