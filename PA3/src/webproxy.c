#include "connection.h"
#include "utils.h"

int main(int argc, char **argv) {
  if (is_cmd_arg_valid(argc, argv) == -1) {
    exit(EXIT_FAILURE);
  }
  int portno = atoi(argv[1]);
  int timeout = atoi(argv[2]);

  int listener_fd, *server_fd;
  socklen_t clientlen = sizeof(struct sockaddr_in);
  struct sockaddr_in clientaddr;
  pthread_t thread_id;

  listener_fd = open_listen_socket(portno);

  while (1) {
    struct thread_params_t *args;
    args = (struct thread_params_t *)malloc(sizeof(struct thread_params_t));
    args->timeout = timeout;
    server_fd = malloc(sizeof(int));
    args->server_fd = *server_fd;
    *server_fd =
        accept(listener_fd, (struct sockaddr *)&clientaddr, &clientlen);
    pthread_create(&thread_id, NULL, proxy_thread, (void *)args);
  }

  return 0;
}