#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "../handshake.h"
#include "../transfer_file.h"
#include "../utils.h"

int is_cmd_arg_valid(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <hostname> <port>\n", argv[0]);
    return -1;
  }

  printf("Num arguments: %d\n", argc);
  printf("Hostname: %s\n", argv[1]);
  printf("Port: %s\n", argv[2]);
  return 1;
}

void print_cmd_prompt() {
  printf("\nType one of the following commands:\n");
  printf("get [filename]\n");
  printf("put [filename]\n");
  printf("delete [filename]\n");
  printf("ls\n");
  printf("exit\n\n");
}

int main(int argc, char **argv) {
  if (is_cmd_arg_valid(argc, argv) == -1) {
    exit(EXIT_FAILURE);
  }

  char *hostname = argv[1];
  int portno = atoi(argv[2]);

  int sockfd;
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) {
    perror("Unable to call socket.");
    exit(EXIT_FAILURE);
  }

  // int status;
  // status = fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0) | O_NONBLOCK);
  // if (status == -1) {
  //   perror("calling fcntl");
  //   exit(EXIT_FAILURE);
  // }

  struct sockaddr_in servaddr;
  socklen_t addrlen = sizeof(struct sockaddr_in);
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(portno);
  inet_aton(hostname, &servaddr.sin_addr);

  int run_program = 1;
  char command_buf[MAXLINE];

  while (run_program == 1) {
    print_cmd_prompt();

    bzero(command_buf, MAXLINE);

    if (fgets(command_buf, sizeof command_buf, stdin)) {
      command_buf[strcspn(command_buf, "\n")] = '\0';
    } else {
      perror("Unable to read line input.");
      continue;
    }

    int option = get_command_option(command_buf);
    if (option == -1) {
      printf("You entered: %s. This command is invalid. Try again.\n",
             command_buf);
      continue;
    }

    printf("Command: %s\n", command_buf);

    // =============================
    // ======== PUT ================
    // =============================

    if (VALID_COMMANDS[option] == "put") {
      char *filename = get_filename(command_buf);
      printf("filename: %s\n", filename);

      int file_desc = open(filename, O_RDWR, S_IRUSR | S_IWUSR | S_IXUSR);
      if (file_desc == -1) {
        perror("File open error. Please use a valid filename.");
        continue;
      }

      int is_hs_ack =
          handshake(command_buf, sockfd, (const struct sockaddr *)&servaddr,
                    sizeof(servaddr), (struct sockaddr *)&servaddr, &addrlen);
      if (is_hs_ack == -1) {
        printf("Unable to execute command\n");
        continue;
      }

      int is_send_done =
          send_file(file_desc, sockfd, (const struct sockaddr *)&servaddr,
                    sizeof(servaddr), (struct sockaddr *)&servaddr, &addrlen);

      if (is_send_done == -1) {
        printf("Unable to send file. Try again.\n");
        continue;
      }

      printf("File sent.\n");
    }

    // =============================
    // ======== GET ================
    // =============================

    else if (VALID_COMMANDS[option] == "get") {
      char *filename = get_filename(command_buf);
      printf("filename: %s\n", filename);
      int file_desc = open(filename, O_CREAT | O_TRUNC | O_RDWR, S_IRWXU);
      if (file_desc == -1) {
        printf("Error opening file. Try again.\n");
        continue;
      }

      int is_hs_ack =
          handshake(command_buf, sockfd, (const struct sockaddr *)&servaddr,
                    sizeof(servaddr), (struct sockaddr *)&servaddr, &addrlen);
      if (is_hs_ack == -1) {
        printf("Unable to execute command\n");
        continue;
      }

      int is_recv_done =
          get_file(file_desc, sockfd, (const struct sockaddr *)&servaddr,
                   sizeof(servaddr), (struct sockaddr *)&servaddr, &addrlen);

      if (is_recv_done == -1) {
        printf("Failed to receive file.\n");
      } else {
        printf("File received.\n");
      }

    }

    // =============================
    // ======== DELETE =============
    // =============================

    else if (VALID_COMMANDS[option] == "delete") {
      int is_hs_ack =
          handshake(command_buf, sockfd, (const struct sockaddr *)&servaddr,
                    sizeof(servaddr), (struct sockaddr *)&servaddr, &addrlen);
      if (is_hs_ack == -1) {
        printf("Unable to delete file. Are you sure it exists?\n");
        continue;
      }
      printf("File deleted.\n");
    }

    // =============================
    // ======== LS =================
    // =============================

    else if (VALID_COMMANDS[option] == "ls") {
      ssize_t is_hs_sent =
          sendto(sockfd, command_buf, MAXLINE, 0,
                 (const struct sockaddr *)&servaddr, sizeof(servaddr));
      if (is_hs_sent == -1) {
        perror("Packet sendto cmd error.");
        continue;
      }

      file_list_t file_list;
      memset(&file_list, 0, sizeof(file_list));
      ssize_t is_recvd = recvfrom(sockfd, &file_list, sizeof(file_list), 0,
                                  (struct sockaddr *)&servaddr, &addrlen);
      if (is_recvd == -1) {
        printf("recvfrom error");
        continue;
      } else if (file_list.status != 0) {
        printf("Unable to receive the list of files. Try again.");
        continue;
      }

      printf("Number of files in the list: %ld\n", file_list.num_files);
      printf("File names: %s\n", file_list.data);
    }

    // =============================
    // ======== EXIT ===============
    // =============================

    else if (VALID_COMMANDS[option] == "exit") {
      int is_hs_ack =
          handshake(command_buf, sockfd, (const struct sockaddr *)&servaddr,
                    sizeof(servaddr), (struct sockaddr *)&servaddr, &addrlen);
      if (is_hs_ack == -1) {
        perror("Unable to execute command. Try again.");
        continue;
      }

      printf("Graceful exit. Good-bye.\n");
      run_program = -1;
    } else {
      perror("Command not recognized. Please try again.\n");
    }
  }

  close(sockfd);
  exit(EXIT_SUCCESS);

  return 0;
}
