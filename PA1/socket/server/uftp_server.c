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

#include "../transfer_file.h"
#include "../utils.h"

int is_cmd_arg_valid(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <port>\n", argv[0]);
    return -1;
  }

  printf("Num arguments: %d\n", argc);
  printf("Port: %s\n", argv[1]);
  return 1;
}

int main(int argc, char **argv) {
  if (is_cmd_arg_valid(argc, argv) == -1) {
    exit(EXIT_FAILURE);
  }

  int port_num = atoi(argv[1]);

  int sockfd;
  struct sockaddr_in servaddr, src_addr;
  socklen_t addrlen = sizeof(struct sockaddr_in);
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
  servaddr.sin_port = htons(port_num);

  // Bind the socket with the server address
  if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  int run_program = 1;
  char command_buf[MAXLINE];

  while (run_program == 1) {
    bzero(command_buf, MAXLINE);

    printf("\nThe server is ready to receive.\n");
    int hs_ack = 0;
    ssize_t is_hs_recvd = recvfrom(sockfd, (char *)command_buf, MAXLINE, 0,
                                   (struct sockaddr *)&src_addr, &addrlen);

    if (is_hs_recvd == -1) {
      printf("is_hs_recvd error");
      continue;
    }

    char *recvd_addr;
    recvd_addr = inet_ntoa(src_addr.sin_addr);
    printf("Msg received from: %s\n", recvd_addr);
    printf("Msg: %s\n", command_buf);

    int option = get_command_option(command_buf);
    if (option == -1) {
      printf("error in get_command_option");
      continue;
    }
    printf("Cmd: %s\n", VALID_COMMANDS[option]);

    // =============================
    // ======== PUT ================
    // =============================

    if (VALID_COMMANDS[option] == "put") {
      char *filename = get_filename(command_buf);
      printf("filename: %s\n", filename);
      int file_desc = open(filename, O_CREAT | O_TRUNC | O_RDWR, S_IRWXU);
      if (file_desc == -1) {
        hs_ack = -1;
        printf("error opening file");
      } else {
        hs_ack = 1;
      }

      ssize_t is_sent =
          sendto(sockfd, &hs_ack, sizeof(hs_ack), 0,
                 (const struct sockaddr *)&src_addr, sizeof(src_addr));
      if (is_sent == -1) {
        perror("sendto error\n");
        exit(EXIT_FAILURE);
      }

      int is_recv_done =
          get_file(file_desc, sockfd, (const struct sockaddr *)&src_addr,
                   sizeof(src_addr), (struct sockaddr *)&src_addr, &addrlen);

      if (is_recv_done == -1) {
        printf("Failed to receive file.\n");
        continue;
      }

      printf("File received.\n");
    }

    // =============================
    // ======== GET ================
    // =============================

    else if (VALID_COMMANDS[option] == "get") {
      char *filename = get_filename(command_buf);
      printf("filename: %s\n", filename);

      int file_desc = open(filename, O_RDWR, S_IRUSR | S_IWUSR | S_IXUSR);
      if (file_desc == -1) {
        perror("File open error.");
        hs_ack = -1;
      } else {
        hs_ack = 1;
      }

      ssize_t is_sent =
          sendto(sockfd, &hs_ack, sizeof(hs_ack), 0,
                 (const struct sockaddr *)&src_addr, sizeof(src_addr));
      if (is_sent == -1) {
        perror("sendto error\n");
        exit(EXIT_FAILURE);
      }

      int is_send_done =
          send_file(file_desc, sockfd, (const struct sockaddr *)&src_addr,
                    sizeof(src_addr), (struct sockaddr *)&src_addr, &addrlen);

      if (is_send_done == -1) {
        printf("Unable to send file. Try again.\n");
        continue;
      }

    }

    // =============================
    // ======== EXIT ===============
    // =============================

    else if (VALID_COMMANDS[option] == "exit") {
      hs_ack = 1;
      ssize_t is_sent =
          sendto(sockfd, &hs_ack, sizeof(hs_ack), 0,
                 (const struct sockaddr *)&src_addr, sizeof(src_addr));
      if (is_sent == -1) {
        perror("sendto error\n");
        exit(EXIT_FAILURE);
      }
      printf("Graceful exit. Good bye.\n");
      run_program = -1;
    }

    // =============================
    // ======== DELETE =============
    // =============================

    else if (VALID_COMMANDS[option] == "delete") {
      char *filename = get_filename(command_buf);
      printf("filename: %s\n", filename);
      int is_rem = remove(filename);

      if (is_rem == 0) {
        printf("File deleted successfully.\n");
        hs_ack = 1;
      } else {
        perror("Delete file error.\n");
        hs_ack = -1;
      }

      ssize_t is_sent =
          sendto(sockfd, &hs_ack, sizeof(hs_ack), 0,
                 (const struct sockaddr *)&src_addr, sizeof(src_addr));
      if (is_sent == -1) {
        perror("sendto error\n");
        exit(EXIT_FAILURE);
      }
    }

    // =============================
    // ======== LS =================
    // =============================

    else if (VALID_COMMANDS[option] == "ls") {
      struct dirent **namelist;
      file_list_t file_list;
      memset(&file_list, 0, sizeof(file_list));

      int file_num = scandir(".", &namelist, filter_dir, alphasort);
      file_list.num_files = file_num;
      printf("Num files: %ld\n", file_list.num_files);

      if (file_num == -1) {
        printf("Error in scandir.");
        file_list.status = 2;
      } else {
        while (file_num--) {
          strcat(file_list.data, namelist[file_num]->d_name);
          strcat(file_list.data, ",");
          free(namelist[file_num]);
        }
        file_list.status = 0;
      }
      free(namelist);
      printf("Sending: %s\n", file_list.data);

      ssize_t is_sent =
          sendto(sockfd, &file_list, sizeof(file_list), 0,
                 (const struct sockaddr *)&src_addr, sizeof(src_addr));
      if (is_sent == -1) {
        perror("sendto error\n");
        exit(EXIT_FAILURE);
      }
    }
  }

  close(sockfd);
  exit(EXIT_SUCCESS);

  return 0;
}
