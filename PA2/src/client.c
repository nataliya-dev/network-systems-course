#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  printf("Hello World!\n");

  return 0;
}

// (echo -en "GET /index.html HTTP/1.1\r\nHost: localhost\r\nConnection:
// Keep-alive\r\n\r\n"; sleep 20; echo -en "GET /index.html HTTP/1.1\r\nHost:
// localhost\r\n\r\n") | telnet 127.0.0.1 9999