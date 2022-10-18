
#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#define MAXLINE 8192 /* max text line length */
#define MAXBUF 8192  /* max I/O buffer size */
#define LISTENQ 1024 /* second argument to listen() */
#define TIMEOUT_S 5

// http://netsys.cs.colorado.edu/

int is_cmd_arg_valid(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <port>\n", argv[0]);
    return -1;
  }
  printf("Port: %s\n", argv[1]);
  return 1;
}

int open_listenfd(int portno) {
  int sockfd, optval = 1;
  struct sockaddr_in servaddr;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1) {
    printf("Socket creation failed...\n");
    exit(EXIT_FAILURE);
  } else {
    printf("Socket successfully created..\n");
  }

  /* Eliminates "Address already in use" error from bind. */
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval,
                 sizeof(int)) < 0) {
    printf("Socket option failed...\n");
    exit(EXIT_FAILURE);
  }

  bzero(&servaddr, sizeof(servaddr));

  // assign IP, PORT
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(portno);

  // Binding newly created socket to given IP and verification
  if ((bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) != 0) {
    printf("Socket bind failed...\n");
    exit(EXIT_FAILURE);
  } else {
    printf("Socket successfully binded..\n");
  }

  /* Make it a listening socket ready to accept connection requests */
  if (listen(sockfd, LISTENQ) < 0) {
    printf("Listening socket failed...\n");
    exit(EXIT_FAILURE);
  }

  return sockfd;
}

typedef struct pair {
  char *key;
  char *value;
} PAIR;

PAIR table[] = {{".css", "text/css"},
                {".gif", "image/gif"},
                {".html", "text/html"},
                {".jpg", "image/jpg"},
                {".js", "application/javascript"},
                {".png", "image/png"},
                {".ico", "image/avif"},
                {".txt", "text/plain"}};

char *get_content_type(char *key) {
  char *ret;
  size_t table_size = sizeof(table) / sizeof(table[0]);
  for (int i = 0; i < table_size; i++) {
    PAIR pair = table[i];
    ret = strstr(key, pair.key);
    if (ret == NULL) {
      continue;
    } else {
      return pair.value;
    }
  }
  return NULL;
}

void send_data(char data_buf[], int connfd, size_t data_total);

void send_error(char data_buf[], int connfd) {
  char *prot = "HTTP/1.1 ";
  char *code = "500 ";
  char *info = "Internal Server Error";

  sprintf(data_buf, "%s%s%s", prot, code, info);
  send_data(data_buf, connfd, strlen(data_buf));
}

void send_data(char data_buf[], int connfd, size_t data_total) {
  size_t bytes_written = write(connfd, data_buf, data_total);
  printf("bytes_written: %ld\n", bytes_written);
  if (bytes_written < 0) {
    printf("ERROR writing message to socket\n");
    send_error(data_buf, connfd);
    return;
  } else if (bytes_written == 0) {
    printf("Zero bytes_written written\n");
    send_error(data_buf, connfd);
    return;
  } else if (bytes_written != data_total) {
    printf("bytes_written and data_total mismatch\n");
    send_error(data_buf, connfd);
    return;
  }
  printf("Done writing\n");
  return;
}

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
    tv.tv_usec = 10;
    setsockopt(connfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof tv);
    printf("No keep alive is requested. Closing connection.\n");
  }
}

void wait_and_receive(int connfd) {
  char command_buf[MAXLINE];
  char data_buf[MAXBUF];
  int keep_alive = 1;

  int rcvd;
  char *method,  // "GET" or "POST"
      *uri,      // "/index.html"
      *prot;     // "HTTP/1.1 or HTTP/1.0"

  while (keep_alive == 1) {
    bzero(command_buf, MAXLINE);
    bzero(data_buf, MAXBUF);

    rcvd = recv(connfd, (char *)command_buf, MAXLINE, 0);
    if (rcvd < 0) {
      printf("recv() timeout\n");
      break;
    } else if (rcvd == 0) {
      printf("client disconnected\n");
      break;
    } else {
      printf("\n+++ New msg recieved from %d +++\n%s", connfd, command_buf);
    }

    // command_buf[rcvd] = '\0';
    char *connection;
    char *pos_keep, *pos_close;
    pos_keep = strstr(command_buf, "keep-alive");
    pos_close = strstr(command_buf, "close");

    if (pos_keep != NULL) {
      // printf("keep-alive found\n");
      connection = "keep-alive";
      keep_alive = 1;
    } else if (pos_close != NULL) {
      // printf("close found\n");
      connection = "close";
      keep_alive = 0;
    } else {
      // printf("neither keep-alive nor close found\n");
      connection = "close";
      keep_alive = 0;
    }

    printf("connection: %s\n", connection);

    method = strtok(command_buf, " \t\r\n");
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

    uri++;
    char *empty = "";
    char file_w_p[MAXLINE];
    char prefix[] = "www/";

    if (*uri == *empty) {
      printf("Loading default page\n");
      uri = "www/index.html";
    } else {
      sprintf(file_w_p, "%s%s", prefix, uri);
      uri = (char *)file_w_p;
    }

    printf("local uri: %s\n", uri);

    if (access(uri, F_OK) == 0) {
      printf("file exists\n");
    } else {
      printf("no such file\n");
      send_error(data_buf, connfd);
      break;
    }

    char *content_type = get_content_type(uri);
    printf("content_type %s\n", content_type);
    if (content_type == NULL) {
      printf("could not find content type\n");
      send_error(data_buf, connfd);
      break;
    }

    struct stat st;
    stat(uri, &st);
    size_t file_size = st.st_size;
    printf("file_size %ld\n", file_size);

    // FILE *fp = fopen(uri, "rb");
    // if (fp == NULL) {
    //   printf("error opening file\n");
    //   send_error(data_buf, connfd);
    //   break;
    // }

    // size_t frame_size = fread(file_data, 1, file_size, fp);
    // printf("frame_size: %ld\n", frame_size);
    // fclose(fp);

    int file_desc = open(uri, O_RDONLY, S_IRUSR);
    if (file_desc == -1) {
      printf("error opening file\n");
      send_error(data_buf, connfd);
      break;
    }

    // fcntl(file_desc, F_SETFL, O_NONBLOCK);

    char *code = " 200";
    char *info = " OK";
    char *c_type = "\r\nContent-Type: ";
    char *c_len = "\r\nContent-Length: ";
    char *c_conn = "\r\nConnection: ";
    char *fc_nl = "\r\n\r\n";

    sprintf(data_buf, "%s%s%s%s%s%s%ld%s%s%s", prot, code, info, c_type,
            content_type, c_len, file_size, c_conn, connection, fc_nl);
    size_t header_total = strlen(data_buf);
    printf("\n=== Header ===\n%s\n", data_buf);
    printf("header_total: %ld\n", header_total);

    size_t frame_num = 1;
    char file_data[MAXBUF - header_total];
    // memset(&file_data, 0, MAXBUF - header_total);
    // memset(&data_buf, 0, MAXBUF);
    // bzero(data_buf, MAXBUF);
    while (1) {
      printf("frame_num: %ld\n", frame_num);
      frame_num++;

      size_t bytes_read = read(file_desc, file_data, MAXBUF - header_total);
      printf("bytes_read: %ld\n", bytes_read);
      // file_data[bytes_read] = '\0';

      if (bytes_read == -1) {
        printf("file read error\n");
        send_error(data_buf, connfd);
        break;
      } else if (bytes_read == 0) {
        printf("file is empty\n");
        break;
      }

      size_t data_total = bytes_read + header_total;
      printf("data_total: %ld\n", data_total);

      // memset(final_buf, 0, data_total);
      char final_buf[data_total];
      sprintf(final_buf, "%s%s", data_buf, file_data);
      // data_buf[data_total] = '\0';

      send_data(final_buf, connfd, data_total);

      bzero(data_buf, MAXBUF);
      bzero(file_data, MAXBUF);
      bzero(final_buf, MAXBUF);

      header_total = 0;
    }

    set_timeout(keep_alive, connfd);
    // if (keep_alive == 0) {
    //   break;
    // }
  }

  printf("Done receiving requests and writing responses.\n");
  return;
}

void *thread(void *vargp) {
  int connfd = *((int *)vargp);
  printf("pthread_detach at %d...\n", connfd);
  pthread_detach(pthread_self());
  free(vargp);
  printf("wait_and_receive at %d ...\n", connfd);
  wait_and_receive(connfd);
  printf("close at %d ...\n", connfd);
  close(connfd);
  printf("exit thread at %d ...\n", connfd);
  return NULL;
}

int main(int argc, char **argv) {
  if (is_cmd_arg_valid(argc, argv) == -1) {
    exit(EXIT_FAILURE);
  }
  int portno = atoi(argv[1]);

  int listenfd = sizeof(struct sockaddr_in);
  socklen_t clientlen;
  struct sockaddr_in clientaddr;
  pthread_t tid;

  listenfd = open_listenfd(portno);

  while (1) {
    int *connfdp = malloc(sizeof(int));
    *connfdp = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);
    pthread_create(&tid, NULL, thread, connfdp);
  }

  return 0;
}
