#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

[[noreturn]] void pdie(char const* fmt, ...) {
  if (errno) perror("die");
  if (fmt) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fputc('\n', stderr);
  }
  exit(EXIT_FAILURE);
}
