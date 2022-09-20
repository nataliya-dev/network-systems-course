#include <arpa/inet.h>
#include <ctype.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT 8001
#define MAXLINE 1024
#define PACKET_SIZE 100
#define RETRY_LIMIT 10

// TODO put these functions in separate file for server and client to share

// Struct that forms a packet to be sent.
typedef struct frame_s {
  char data[PACKET_SIZE];
  size_t seq;
  size_t size;
} frame_t;

char *duplicate_str(const char *str) {
  char *newstr = (char *)malloc(strlen(str) + 1);
  if (newstr) {
    strcpy(newstr, str);
  }
  return newstr;
}

char *VALID_COMMANDS[5] = {"get", "put", "delete", "ls", "exit"};
int get_command_option(const char *str) {
  char *token = strtok(duplicate_str(str), " ");
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
  char *token = strtok(duplicate_str(str), " ");
  token = strtok(NULL, " ");
  return token;
}

int main() {
  int sockfd;
  struct sockaddr_in servaddr, src_addr;

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
  char rcvbuf[MAXLINE];
  printf("The server is ready to receive\n");
  int hs_ack = 0;
  ssize_t is_hs_recvd = recvfrom(sockfd, (char *)rcvbuf, MAXLINE, 0,
                                 (struct sockaddr *)&src_addr, &addrlen);

  // TODO do not move on to next error if previous failes, just move to negative
  // ack
  if (is_hs_recvd == -1) {
    perror("is_hs_recvd error");
  }

  char *recvd_addr;
  recvd_addr = inet_ntoa(src_addr.sin_addr);
  printf("Msg received from: %s\n", recvd_addr);
  printf("Msg: %s\n", rcvbuf);

  int option = get_command_option(rcvbuf);
  if (option == -1) {
    printf("error in get_command_option");
  }
  printf("Cmd: %s\n", VALID_COMMANDS[option]);

  if (VALID_COMMANDS[option] == "put") {
    char *filename = get_filename(rcvbuf);
    printf("filename: %s\n", filename);
    int file_desc = open(filename, O_CREAT | O_TRUNC | O_RDWR, S_IRWXU);
    if (file_desc == -1) {
      perror("error opening file");
    } else {
      hs_ack = 1;
    }

    ssize_t is_sent =
        sendto(sockfd, &hs_ack, sizeof(hs_ack), 0,
               (const struct sockaddr *)&src_addr, sizeof(src_addr));
    if (is_sent == -1) {
      perror("sendto error");
      exit(EXIT_FAILURE);
    }

    int is_recv_done = 0;
    frame_t frame;

    while (is_recv_done == 0) {
      memset(&frame, 0, sizeof(frame));

      ssize_t is_recvd = recvfrom(sockfd, &frame, sizeof(frame), 0,
                                  (struct sockaddr *)&src_addr, &addrlen);
      sendto(sockfd, &frame.seq, sizeof(frame.seq), 0,
             (const struct sockaddr *)&src_addr, sizeof(src_addr));

      if (frame.seq == 0) {
        is_recv_done = 1;
      }

      ssize_t is_written = write(file_desc, frame.data, frame.size);
      if (is_written == -1) {
        perror("write error");
        break;
      }
    }
    close(file_desc);

    if (is_recv_done == 0) {
      perror("Failed to receive file.");
      exit(EXIT_FAILURE);
    }

    printf("File received.\n");
  }

  close(sockfd);

  return 0;
}
