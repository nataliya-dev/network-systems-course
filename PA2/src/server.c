
#include <arpa/inet.h>
#include <ctype.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAXLINE 8192
#define MAXBUF 8192
#define LISTEBUF 1024
#define POST_MAX 100
#define TIMEOUT_S 20

/**
 * Inform the user of the interface.
 */
int is_cmd_arg_valid(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <port>\n", argv[0]);
    return -1;
  }
  printf("Port: %s\n", argv[1]);
  return 1;
}

/**
 * Create a passivee socket that accepts incoming connections.
 */
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
    printf("Socket option failed...\n");
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

  if (listen(socket_fd, LISTEBUF) <
      0) {  // passive socket that accepts connections
    printf("Listening socket failed...\n");
    exit(EXIT_FAILURE);
  }

  return socket_fd;
}

/**
 * Create a one-to-one mapping between MIME types and extension.
 */
typedef struct pair {
  char *extension;
  char *mime;
} PAIR;

PAIR table[] = {{".css", "text/css"},
                {".gif", "image/gif"},
                {".html", "text/html"},
                {".jpg", "image/jpg"},
                {".js", "application/javascript"},
                {".png", "image/png"},
                {".txt", "text/plain"},
                {".ico", "image/avif"}};

/**
 * Obtain the MIME type based on the input.
 */
char *get_content_type(char *key) {
  char *ret;
  size_t table_size = sizeof(table) / sizeof(table[0]);
  for (int i = 0; i < table_size; i++) {
    PAIR pair = table[i];
    ret = strstr(key, pair.extension);
    if (ret == NULL) {
      continue;
    } else {
      return pair.mime;
    }
  }
  return NULL;
}

/**
 * If anything goes wrong, this is the go-to error to send back to the client.
 */
void send_error(char data_buf[], int connfd) {
  bzero(data_buf, MAXBUF);

  char *prot = "HTTP/1.1 ";
  char *code = "500 ";
  char *info = "Internal Server Error\r\n";

  sprintf(data_buf, "%s%s%s", prot, code, info);
  printf("%s\n", data_buf);
  size_t bytes_written = write(connfd, data_buf, strlen(data_buf));
  printf("bytes_written: %ld\n", bytes_written);
}

/**
 * Sending requested data to the client.
 */
void send_data(char data_buf[], int connfd, size_t data_total) {
  size_t bytes_written = write(connfd, data_buf, data_total);
  printf("bytes_written: %ld\n", bytes_written);

  if (bytes_written < 0) {
    printf("ERROR bytes_written < 0\n");
    send_error(data_buf, connfd);
    return;
  } else if (bytes_written == 0) {
    printf("ERROR bytes_written == 0\n");
    send_error(data_buf, connfd);
    return;
  } else if (bytes_written != data_total) {
    printf("ERROR bytes_written != data_total\n");
    send_error(data_buf, connfd);
    return;
  }
  printf("Done writing\n");
  return;
}

/**
 * If the user requests to keep the connectin alive then we set the receive
 * socket to wait for a message for a given amount of time before exiting and
 * closing the connection.
 */
void set_timeout(int keep_alive, int connfd) {
  struct timeval tv;
  tv.tv_usec = 0;
  if (keep_alive == 1) {
    tv.tv_sec = TIMEOUT_S;
    tv.tv_usec = 0;
    setsockopt(connfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof tv);
    printf("Keep alive is requested. Checking for another msg.\n");
  } else {
    tv.tv_sec = 0;
    tv.tv_usec = 1;
    setsockopt(connfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof tv);
    printf("No keep alive is requested. Closing connection.\n");
  }
}

/**
 * Duplicate the input string for reuse. Helps isolate some operations.
 */
char *duplicate_str(const char *str) {
  char *newstr = (char *)malloc(strlen(str) + 1);
  if (newstr) {
    strcpy(newstr, str);
  }
  return newstr;
}

/**
 * Check whether the client wants the server to keep the connection open or to
 * close it after having received the message.
 */
char *get_connection(char *command_buf, int *keep_alive) {
  char *pos_keep, *pos_close, *connection;
  char *command_buf_lower = duplicate_str(command_buf);
  for (size_t i = 0; i < strlen(command_buf_lower); i++) {
    command_buf_lower[i] = tolower(command_buf_lower[i]);
  }
  pos_keep = strstr(command_buf_lower, "keep-alive");
  pos_close = strstr(command_buf_lower, "close");
  if (pos_keep != NULL) {
    connection = "keep-alive";
    *keep_alive = 1;
  } else if (pos_close != NULL) {
    connection = "close";
    *keep_alive = 0;
  } else {
    connection = "close";
    *keep_alive = 0;
  }
  return connection;
  printf("connection: %s\n", connection);
}

/**
 * The main workhorse of the server. This is where all the data gets processed
 * to understand what the server wants.
 */
void exchange_data(int connfd) {
  char command_buf[MAXLINE];
  char data_buf[MAXBUF];
  int keep_alive = 1;
  char *method, *uri, *prot;

  while (keep_alive == 1) {
    bzero(command_buf, MAXLINE);
    bzero(data_buf, MAXBUF);

    int rec = recv(connfd, command_buf, MAXLINE, 0);
    if (rec < 0) {
      break;
    } else if (rec == 0) {
      break;
    }
    printf("\n+++ New msg recieved from %d +++\n", connfd);
    printf("%s\n", command_buf);

    char *connection = get_connection((char *)command_buf, &keep_alive);
    printf("connection: %s\n", connection);

    char *command_buf_cp = duplicate_str(command_buf);
    method = strtok(command_buf_cp, " \t\r\n");
    uri = strtok(NULL, " \t\r\n");
    prot = strtok(NULL, " \t\r\n");

    printf("method: %s\n", method);
    printf("uri: %s\n", uri);
    printf("prot: %s\n", prot);

    if (method == NULL || uri == NULL || prot == NULL) {
      printf("Unable to parse message\n");
      send_error(data_buf, connfd);
      break;
    }

    char *post = "POST";
    char *get = "GET";
    char *post_msg;
    if (*method == *post) {
      printf("POST request\n");
      size_t i = 0;
      for (; i < strlen(command_buf); i++) {
        char c_from_buf = command_buf[i];
        if (i >= 4 && c_from_buf == '\n' &&
            command_buf[i - 2] == '\n') {  // two new lines in a row indicate
                                           // the end of the header
          break;
        }
      }
      post_msg = command_buf + i + 1;
      printf("post_msgs: %s\n", post_msg);
    } else if (*method == *get) {
      printf("GET request\n");
    } else {
      printf("Request method not recognized.\n");
      send_error(data_buf, connfd);
      break;
    }

    uri++;  // not include the '/' charachter
    char *empty = "";
    char *prefix = "www/";
    char loca_uri[strlen(prefix) + strlen(uri)];

    if (*uri == *empty) {
      printf("Loading default page\n");
      uri = "www/index.html";
    } else {
      sprintf(loca_uri, "%s%s", prefix, uri);
      uri = loca_uri;
    }
    printf("loca_uri: %s\n", uri);  // the full file path from root

    if (access(uri, F_OK) == 0) {  // check if we can open the file
      printf("file exists\n");
    } else {
      printf("no such file\n");
      send_error(data_buf, connfd);
      break;
    }

    char *content_type = get_content_type(uri);
    printf("content_type: %s\n", content_type);
    if (content_type == NULL) {
      printf("could not find content type\n");
      send_error(data_buf, connfd);
      break;
    }

    struct stat st;
    stat(uri, &st);
    size_t file_size = st.st_size;
    printf("file_size %ld\n", file_size);

    int file_desc = open(uri, O_RDONLY, S_IRUSR);
    if (file_desc == -1) {
      printf("error opening file\n");
      send_error(data_buf, connfd);
      break;
    }

    char *is_html = strstr(content_type, "text/html");
    char post_buf[POST_MAX];
    if (is_html != NULL && post_msg != NULL) {
      sprintf(post_buf, "%s%s%s", "<html><body><pre><h1>", post_msg,
              "</h1></pre></html>");
    }
    printf("post_buf: %s\n", post_buf);

    char *code = " 200 ";  // info to put together a header
    char *info = "OK";
    char *c_type = "\r\nContent-Type: ";
    char *c_len = "\r\nContent-Length: ";
    char *c_conn = "\r\nConnection: ";
    char *fc_nl = "\r\n\r\n";

    bzero(data_buf, MAXBUF);
    sprintf(data_buf, "%s%s%s%s%s%s%ld%s%s%s%s", prot, code, info, c_type,
            content_type, c_len, file_size, c_conn, connection, fc_nl,
            post_buf);
    size_t header_total = strlen(data_buf);
    printf("header_total: %ld\n", header_total);
    printf("\n=== Header ===\n%s\n", data_buf);

    while (1) {  // send data in chunks as allowed by the size of the buffer
      size_t frame_size =
          read(file_desc, data_buf + header_total, MAXBUF - header_total);
      printf("frame_size: %ld\n", frame_size);
      if (frame_size == -1) {
        printf("frame_size == -1, file read error\n");
        send_error(data_buf, connfd);
        break;
      } else if (frame_size == 0) {
        printf("frame_size == 0, reached end-of-file\n");
        break;
      }

      size_t data_total = header_total + frame_size;
      printf("data_total: %ld\n", data_total);

      send_data(data_buf, connfd, data_total);
      bzero(data_buf, MAXBUF);
      header_total = 0;
    }

    set_timeout(keep_alive, connfd);  // keep the connection alive if requested
  }

  printf("Done with data exchange with %d\n", connfd);
  return;
}

void *server_thread(void *vargp) {
  int connfd = *((int *)vargp);
  printf("pthread_detach %d\n", connfd);
  pthread_detach(pthread_self());
  free(vargp);
  printf("exchange_data %d\n", connfd);
  exchange_data(connfd);
  printf("close %d\n", connfd);
  close(connfd);
  printf("exit thread %d\n", connfd);
  return NULL;
}

int main(int argc, char **argv) {
  if (is_cmd_arg_valid(argc, argv) == -1) {
    exit(EXIT_FAILURE);
  }
  int portno = atoi(argv[1]);

  int listener_fd, *server_fd;
  socklen_t clientlen = sizeof(struct sockaddr_in);
  struct sockaddr_in clientaddr;
  pthread_t thread_id;

  listener_fd = open_listen_socket(portno);

  while (1) {
    server_fd = malloc(sizeof(int));
    *server_fd =
        accept(listener_fd, (struct sockaddr *)&clientaddr, &clientlen);
    pthread_create(&thread_id, NULL, server_thread, server_fd);
  }

  return 0;
}
