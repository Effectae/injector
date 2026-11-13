#pragma once

#include <sys/types.h>

typedef struct Dl_fn {
  size_t dlopen, dlsym;
} Dl_fn;

bool dl_fn_from(Dl_fn* dl, char const* elf_path);
