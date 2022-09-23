#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "../utils/handshake.h"
#include "../utils/transfer_file.h"
#include "../utils/utils.h"

#define DO_CLIENT_EXIT \
  1  // whether or not the client should gracefully exit along with the server.

/* Check whether or not the input arguments satisfy program requirements. The
 * hostname and port must be specified. */
int is_cmd_arg_valid(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <hostname> <port>\n", argv[0]);
    return -1;
  }

  // printf("Num arguments: %d\n", argc);
  printf("Hostname: %s\n", argv[1]);
  printf("Port: %s\n", argv[2]);
  return 1;
}

/* Instructions for the user on how to use the client program. */
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

  /* Create an endpoint for communication and specify the domain, type and
   * protocol. */
  int sockfd;
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  /*Returns a file descriptor on success and -1 on failure. */
  if (sockfd == -1) {
    perror("Unable to call socket.\n");
    exit(EXIT_FAILURE);
  }

  /* Fill out a struct for handling internet addresses. User inputs are used
   * here.*/
  struct sockaddr_in servaddr;
  socklen_t addrlen = sizeof(struct sockaddr_in);
  memset(&servaddr, 0,
         sizeof(servaddr));                 // initialize the block of mem
  servaddr.sin_family = AF_INET;            // IPv4 Internet protocols
  servaddr.sin_port = htons(portno);        // convert to network byte order
  inet_aton(hostname, &servaddr.sin_addr);  // from ipv4 to byte form

  int run_program = 1;        // controls the outer while loop
  char command_buf[MAXLINE];  // Stores user command input

  while (run_program == 1) {
    print_cmd_prompt();  // prompt user for input

    bzero(command_buf, MAXLINE);  // clear memory for input

    /* Read line from command line stream and store it.*/
    if (fgets(command_buf, sizeof command_buf, stdin)) {
      command_buf[strcspn(command_buf, "\n")] = '\0';
    } else {
      printf("Unable to read line input.\n");
      continue;
    }

    /* Check to see which command has been selected.*/
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
      /* Retrieve the filename from the stream.*/
      char *filename = get_filename(command_buf);
      printf("filename: %s\n", filename);

      /* Check to see if this is a valid file.*/
      int file_desc = open(filename, O_RDWR, S_IRUSR | S_IWUSR | S_IXUSR);
      if (file_desc == -1) {
        printf("File open error. Please use a valid filename.\n");
        continue;
      }

      /* Convey command to the server and ask for acknowledgement. See utils
       * folder for implementation.*/
      int is_hs_ack =
          handshake(command_buf, sockfd, (const struct sockaddr *)&servaddr,
                    sizeof(servaddr), (struct sockaddr *)&servaddr, &addrlen);
      if (is_hs_ack == -1) {
        printf("Unable to execute command. Try again.\n");
        continue;
      }

      /* Send the file realiably, see utils folder for implementation.*/
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
      /* Retrieve the filename from the stream.*/
      char *filename = get_filename(command_buf);
      printf("filename: %s\n", filename);
      int file_desc = open(filename, O_CREAT | O_TRUNC | O_RDWR, S_IRWXU);
      if (file_desc == -1) {
        printf("Error opening file. Try again.\n");
        continue;
      }

      /* Convey command to the server and ask for acknowledgement. See utils
       * folder for implementation.*/
      int is_hs_ack =
          handshake(command_buf, sockfd, (const struct sockaddr *)&servaddr,
                    sizeof(servaddr), (struct sockaddr *)&servaddr, &addrlen);
      if (is_hs_ack == -1) {
        printf("Unable to execute command.\n");
        continue;
      }

      /* Get the file reliably, see utils folder for implementation.*/
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
      /* Send the delete command to the server.*/
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
      /* Send the ls command to the server.*/
      ssize_t is_hs_sent =
          sendto(sockfd, command_buf, MAXLINE, 0,
                 (const struct sockaddr *)&servaddr, sizeof(servaddr));
      if (is_hs_sent == -1) {
        printf("Packet sendto cmd error.\n");
        continue;
      }

      /* Recieve the list of files. They are comma separated. Stored in a
       * struct so the client knows how many files should be in the data.*/
      file_list_t file_list;
      memset(&file_list, 0, sizeof(file_list));
      ssize_t is_recvd = recvfrom(sockfd, &file_list, sizeof(file_list), 0,
                                  (struct sockaddr *)&servaddr, &addrlen);
      if (is_recvd == -1) {
        printf("recvfrom error\n");
        continue;
      } else if (file_list.status != 0) {
        printf("Unable to receive the list of files. Try again.\n");
        continue;
      }

      printf("Number of files in the list: %ld\n", file_list.num_files);
      printf("File names: %s\n", file_list.data);
    }

    // =============================
    // ======== EXIT ===============
    // =============================

    else if (VALID_COMMANDS[option] == "exit") {
      /* Send the exit command to the server.*/
      int is_hs_ack =
          handshake(command_buf, sockfd, (const struct sockaddr *)&servaddr,
                    sizeof(servaddr), (struct sockaddr *)&servaddr, &addrlen);
      if (is_hs_ack == -1) {
        perror("Unable to execute command. Try again.\n");
        continue;
      }

      printf("Graceful exit.\n");

      if (DO_CLIENT_EXIT) {
        printf("Goodbye!");
        run_program = -1;
      }

    } else {
      perror("Command not recognized. Please try again.\n");
    }
  }

  close(sockfd);
  exit(EXIT_SUCCESS);

  return 0;
}
