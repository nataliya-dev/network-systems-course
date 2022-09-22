#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MAXLINE 1024
#define PACKET_SIZE 100
#define RETRY_LIMIT 10

typedef struct file_list_s {
  char data[MAXLINE];
  size_t num_files;
  size_t status;  // 0=good, 2=bad
} file_list_t;

char *duplicate_str(const char *str) {
  char *newstr = (char *)malloc(strlen(str) + 1);
  if (newstr) {
    strcpy(newstr, str);
  }
  return newstr;
}

char *VALID_COMMANDS[5] = {"get", "put", "delete", "ls", "exit"};
int get_command_option(const char *str) {
  char *token = strtok(duplicate_str(str), " ");
  int len = sizeof(VALID_COMMANDS) / sizeof(VALID_COMMANDS[0]);
  int i = 0;
  for (i = 0; i < len; ++i) {
    if (strcmp(VALID_COMMANDS[i], token) == 0) {
      return i;
    }
  }
  return -1;
}

char *get_filename(const char *str) {
  char *token = strtok(duplicate_str(str), " ");
  token = strtok(NULL, " ");
  return token;
}

int filter_dir(const struct dirent *e) {
  struct stat st;
  stat(e->d_name, &st);
  return !(st.st_mode & S_IFDIR);
}
