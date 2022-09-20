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
#define PACKET_SIZE 100
#define RETRY_LIMIT 10

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

  char command_buf[MAXLINE];
  if (fgets(command_buf, sizeof command_buf, stdin)) {
    command_buf[strcspn(command_buf, "\n")] = '\0';
  }

  int option = get_command_option(command_buf);
  if (option == -1) {
    printf("%s command is invalid. Try again.\n", command_buf);
  }

  printf("Msg: %s\n", command_buf);

  // TODO timeout for send and receive

  // initial handshake
  ssize_t is_hs_sent =
      sendto(sockfd, command_buf, MAXLINE, 0,
             (const struct sockaddr *)&servaddr, sizeof(servaddr));
  if (is_hs_sent == -1) {
    perror("packet sendto cmd error");
    exit(EXIT_FAILURE);
  }

  printf("Waiting for ack handshake.\n");

  // receive handshake response
  size_t is_hs_ack = 0;
  socklen_t addrlen = sizeof(struct sockaddr_in);
  ssize_t is_hs_recvd = recvfrom(sockfd, &is_hs_ack, sizeof(is_hs_ack), 0,
                                 (struct sockaddr *)&servaddr, &addrlen);
  if (is_hs_recvd == -1) {
    perror("is_hs_recvd error");
    exit(EXIT_FAILURE);
  } else if (is_hs_ack != 1) {
    perror("is_hs_ack error");
    exit(EXIT_FAILURE);
  } else {
    printf("Handshake success!\n");
  }

  if (VALID_COMMANDS[option] == "put") {
    char *filename = get_filename(command_buf);
    // char *filename = "foo1.jpg";
    printf("filename: %s\n", filename);

    int file_desc = open(filename, O_RDWR, S_IRUSR | S_IWUSR | S_IXUSR);
    if (file_desc == -1) {
      perror("file open error");
      exit(EXIT_FAILURE);
    }

    frame_t frame;
    size_t seq_num = 1;
    size_t ack = 0;
    int is_send_done = 0;

    while (is_send_done == 0) {
      memset(&frame, 0, sizeof(frame));

      frame.seq = seq_num;

      frame.size = read(file_desc, frame.data, PACKET_SIZE);
      printf("frame.seq %ld, frame.size %ld\n", frame.seq, frame.size);
      if (frame.size == -1) {
        perror("file read error");
        return -1;
      }

      if (frame.size == 0) {
        frame.seq = 0;
        is_send_done = 1;
      }

      size_t retries = 0;
      int is_acked = 0;

      while (is_acked == 0 && retries <= RETRY_LIMIT) {
        ssize_t is_sent =
            sendto(sockfd, &frame, sizeof(frame), 0,
                   (const struct sockaddr *)&servaddr, sizeof(servaddr));
        if (is_sent == -1) {
          printf("packet sendto error, frame.seq %ld, frame.size %ld\n",
                 frame.seq, frame.size);
          retries++;
          continue;
        }

        ssize_t is_recvd = recvfrom(sockfd, &ack, sizeof(ack), 0,
                                    (struct sockaddr *)&servaddr, &addrlen);
        if (is_recvd == -1) {
          printf("packet recvfrom error, frame.seq %ld, frame.size %ld\n",
                 frame.seq, frame.size);
          retries++;
          continue;
        } else if (frame.seq != ack) {
          printf("packet seq mismatch error, frame.seq %ld, frame.size %ld\n",
                 frame.seq, frame.size);
          retries++;
          continue;
        }

        is_acked = 1;
      }

      if (is_acked == 0) {
        perror("unable to send packet");
        break;
      }

      seq_num++;
    }

    close(file_desc);

    if (is_send_done == 0) {
      perror("unable to send packet");
      exit(EXIT_FAILURE);
    }

    printf("File sent.");
  }

  close(sockfd);
  return 0;
}
