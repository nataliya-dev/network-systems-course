#include "utils.h"

int is_cmd_arg_valid(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <port> <timeout>\n", argv[0]);
    return -1;
  }
  printf("Port: %s\n", argv[1]);
  printf("Timeout: %s\n", argv[2]);
  return 1;
}

char *duplicate_str(const char *str) {
  char *newstr = (char *)malloc(strlen(str) + 1);
  if (newstr) {
    strcpy(newstr, str);
  }
  return newstr;
}

char *replace_char(char *str, char find, char replace) {
  char *dup = duplicate_str(str);
  char *current_pos = strchr(dup, find);
  while (current_pos) {
    *current_pos = replace;
    current_pos = strchr(current_pos, find);
  }
  return dup;
}

char *encode_file_name(char *str) {
  char *file_name = replace_char(str, '/', '?');
  return file_name;
}
char *decode_file_name(char *str) {
  char *file_name = replace_char(str, '?', '/');
  return file_name;
}