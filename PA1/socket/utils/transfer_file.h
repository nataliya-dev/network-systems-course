
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

#define PACKET_SIZE 100
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
  int is_recv_done = -1;
  frame_t frame;
  size_t prev_seq = 0;

  while (is_recv_done == -1) {
    memset(&frame, 0, sizeof(frame));

    ssize_t is_recvd =
        recvfrom(sockfd, &frame, sizeof(frame), 0, src_addr, src_addrlen);
    printf("frame.seq %ld, frame.size %ld\n", frame.seq, frame.size);

    if (is_recvd == -1) {
      printf("recvfrom error. Try again.\n");
      break;
    } else if (frame.status == 2) {
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

  while (is_send_done == -1) {
    memset(&frame, 0, sizeof(frame));
    frame.status = 0;
    frame.seq = seq_num;

    frame.size = read(file_desc, frame.data, PACKET_SIZE);
    printf("frame.seq %ld, frame.size %ld\n", frame.seq, frame.size);
    if (frame.size == -1) {
      perror("file read error");
      frame.status = 2;
    } else if (frame.size == 0) {
      frame.status = 1;
      is_send_done = 1;
    }

    size_t retries = 0;
    int is_acked = 0;

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

  close(file_desc);
  return is_send_done;
}

#endif