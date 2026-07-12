#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
  // TODO: free buffer
  size_t buf_len = 128;
  char *buf = malloc(buf_len);
  if (buf == NULL) {
    printf("[ERROR] unable to allocate buffer\n");
    return 1;
  }

  size_t argvec_c = 32;
  char **args = malloc(argvec_c * sizeof(char*));
  if (args == NULL) {
    printf("[ERROR] unable to allocate args buffer\n");
    return 1;
  }

  while (1) {
    printf("prompt> ");
    ssize_t cmd_len = getline(&buf, &buf_len, stdin);
    if (cmd_len == -1) {
      printf("\n");
      if (feof(stdin)) {
        return 0;
      }

      printf("[ERROR] getline stat -1\n");
      return 1;
    }

    if (buf[cmd_len-1] == '\n') {
      buf[cmd_len-1] = '\0';
    }

    char *token;
    char *strp = buf;

    int wc = 0;
    while ((token = strsep(&strp, " ")) != NULL) {
      // skip empty tokens
      if (*token == '\0') {
        continue;
      }

      // expand args buffer if necessary
      if (wc == argvec_c) {
        argvec_c *= 2;
        args = realloc(args, argvec_c * sizeof(char*));
        if (args == NULL) {
          printf("[ERROR] unable to reallocate args buffer\n");
          return 1;
        }
      }

      args[wc] = token;
      wc++;
    }
    // null terminate args array
    if (wc == argvec_c) {
      argvec_c *= 2;
      args = realloc(args, argvec_c * sizeof(char*));
      if (args == NULL) {
        printf("[ERROR] unable to reallocate args buffer\n");
        return 1;
      }
    }
    args[wc] = NULL;

    // ignore empty lines
    if (wc == 0) {
      continue;
    }

    int pid = fork();
    if (pid == -1) {
      printf("[ERROR] fork failed\n");
      return 1;
    }
    else if (pid == 0) {
      // child proc
      if (execvp(args[0], args) == -1) {
        if (errno == ENOENT) {
          printf("%s: command not found\n", args[0]);
          return 2;
        }
        perror("[ERROR] exec");
        return 124;
      }
    }
    else {
      // parent proc
      int status;
      wait(&status);
      printf("%d ", status);
    } 
  }
}
