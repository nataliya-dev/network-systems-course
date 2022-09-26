
#ifndef NN_TRANSFER_FILE_H
#define NN_TRANSFER_FILE_H

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

#define PACKET_SIZE 1000
#define NUM_RETRIES 10

typedef struct frame_s {
  char data[PACKET_SIZE];  // data being transferred
  size_t seq;              // sequence number of the packet
  size_t size;             // the size of the packet being sent
  size_t status;           // 0=good, 1=done, 2=abandon
} frame_t;

int get_file(int file_desc, int sockfd, const struct sockaddr *dest_addrs,
             socklen_t dest_addrlen, struct sockaddr *src_addr,
             socklen_t *src_addrlen) {
  int is_recv_done = -1;  // loop control
  frame_t frame;          // frame to be transferred
  size_t prev_seq = 0;    // keep track of the file sequence ack

  while (is_recv_done == -1) {
    memset(&frame, 0, sizeof(frame));

    ssize_t is_recvd =
        recvfrom(sockfd, &frame, sizeof(frame), 0, src_addr, src_addrlen);
    printf("frame.seq %ld, frame.size %ld\n", frame.seq, frame.size);

    if (is_recvd == -1) {
      printf("recvfrom error. Try again.\n");
      break;
    } else if (frame.status == 2) {
      // this status means to abandon the send mission
      printf("server error. Try again.\n");
      break;
    }

    ssize_t is_sent = sendto(sockfd, &frame.seq, sizeof(frame.seq), 0,
                             dest_addrs, dest_addrlen);

    if (is_sent == -1) {
      printf("sendto error\n");
      break;
    } else if (frame.status == 1) {
      is_recv_done = 1;
    }

    if (frame.seq <= prev_seq) {
      printf("Out of seq frame received.");
      continue;
    }

    ssize_t is_written = write(file_desc, frame.data, frame.size);
    if (is_written == -1) {
      printf("Write error. Try again.\n");
      break;
    }
    prev_seq = frame.seq;
  }
  close(file_desc);
  return is_recv_done;
}

int send_file(int file_desc, int sockfd, const struct sockaddr *dest_addrs,
              socklen_t dest_addrlen, struct sockaddr *src_addr,
              socklen_t *src_addrlen) {
  frame_t frame;
  size_t seq_num = 1;
  size_t ack = 0;
  int is_send_done = -1;

  /*Add a one second timeout for file receipt. Retry when no ack received.*/
  struct timeval time_val;
  time_val.tv_sec = 1;
  time_val.tv_usec = 0;
  if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &time_val, sizeof(time_val)) <
      0) {
    perror("setsockopt error");
  }

  while (is_send_done == -1) {
    memset(&frame, 0, sizeof(frame));
    frame.status = 0;
    frame.seq = seq_num;

    frame.size = read(file_desc, frame.data, PACKET_SIZE);
    printf("frame.seq %ld, frame.size %ld\n", frame.seq, frame.size);
    if (frame.size == -1) {
      perror("file read error");
      frame.status = 2;
      // this status means abandon the receive mission
    } else if (frame.size == 0) {
      frame.status = 1;
      is_send_done = 1;
    }

    size_t retries = 0;
    int is_acked = 0;

    /* Keep trying to send the file until the tries have been exhausted.*/
    while (is_acked == 0 && retries <= NUM_RETRIES) {
      ssize_t is_sent =
          sendto(sockfd, &frame, sizeof(frame), 0, dest_addrs, dest_addrlen);
      if (is_sent == -1) {
        retries++;
        continue;
      }

      ssize_t is_recvd =
          recvfrom(sockfd, &ack, sizeof(ack), 0, src_addr, src_addrlen);
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

  /*After execution set the timeout back to zero, which means that recvfrom will
   * be a blocking call.*/
  time_val.tv_sec = 0;
  time_val.tv_usec = 0;
  if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &time_val, sizeof(time_val)) <
      0) {
    perror("Error");
  }

  close(file_desc);
  return is_send_done;
}

#endif