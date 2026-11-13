#pragma once

#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>

typedef struct ProcMemRegion {
  uintptr_t start, end;

  off_t offset;

  char perm[5];
} ProcMemRegion;

typedef struct ProcMap {

  char *name;
  size_t count;
  ProcMemRegion regions[];
} ProcMap;

// find libc.so or libdl.so
ProcMap *FindLibMapFromPid(FILE *map_fp, char const *lib, char **line_buf,
                           size_t *buf_size);
void DropProcMap(ProcMap *p);
