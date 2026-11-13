#include "proc.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "utils.h"

size_t string_starts_with(char const* str, char const* start) {
  char const* old_str = str;
  while (*start)
    if (*str++ != *start++) return 0;
  return str - old_str;
}

static void parse_region(ProcMemRegion* region, char const* str) {
  sscanf(str, "%" SCNxPTR "-%" SCNxPTR " %4s %lx %*s",  // NOLINT

         &region->start, &region->end, region->perm, &region->offset

  );
}

static bool is_lib(char const* lib_base_name, char const* path) {
  const char* p = strrchr(path, '/');
  if (!p)
    p = path;
  else
    p += 1;
  if (strncmp(p, "lib", 3) != 0) return false;
  p += 3;
  size_t len = string_starts_with(p, lib_base_name);
  if (!len) return false;
  p += len;
  if (strncmp(p, ".so", 3) != 0) return false;
  p += 3;

  char c;
  for (int i = 0; i < 3; i++) {
    c = *p;
    if (c == '\0' || c == '\n') return true;
    if (c != '.') return false;
    c = *++p;

    if (!('0' <= c && c <= '9')) return false;

    do {
      c = *++p;
    } while (!('0' <= c && c <= '9'));
  }
  return (*p == '\0' || *p == '\n') != 0;
}

static ProcMap* lib_map_from(char const* lib, char** line_buf, size_t* buf_size, FILE* fp,
                             size_t idx) {
  ProcMap* ret;

  ssize_t size = getline(line_buf, buf_size, fp);
  if (size < 0) {
    ret = malloc(sizeof(*ret) + (idx * sizeof(ret->regions[0])));
    if (!ret) pdie(nullptr);
    ret->count = idx;
    return ret;
  }

  const char* p = strchr(*line_buf, '/');
  if (!p || !string_starts_with(p, lib)) return lib_map_from(lib, line_buf, buf_size, fp, idx);

  ProcMemRegion region;
  parse_region(&region, *line_buf);
  ret = lib_map_from(lib, line_buf, buf_size, fp, idx + 1);
  ret->regions[idx] = region;
  return ret;
}

ProcMap* FindLibMapFromPid(FILE* fp, char const* lib_base_name, char** line_buf, size_t* buf_size) {
  ProcMap* ret = nullptr;
  ssize_t size;

  while (0 < (size = getline(line_buf, buf_size, fp))) {
    char const* p = strchr(*line_buf, '/');
    if (!p) continue;
    if (is_lib(lib_base_name, p)) {
      (*line_buf)[size - 1] = '\0';
      char* lib_name = strdup(p);
      if (!lib_name) pdie(nullptr);
      ProcMemRegion region;
      parse_region(&region, *line_buf);
      ret = lib_map_from(lib_name, line_buf, buf_size, fp, 1);
      ret->name = lib_name;
      *ret->regions = region;
      break;
    }
  }
  return ret;
}

void DropProcMap(ProcMap* p) {
  if (!p) return;
  free(p->name);
  free(p);
}
