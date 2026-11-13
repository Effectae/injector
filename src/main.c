#include <__stddef_unreachable.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "utils.h"

#ifndef INJECTOR_VERSION
#define INJECTOR_VERSION "UNDEFINED"
#endif

static const char version[] = INJECTOR_VERSION;

static const char help_text[] =

    "Usage: Inject [option...]\n"
    "OPTIONS\n"
    "\t-h\tPrint the summary\n"
    "\t-p\tProcess id to be injected\n"
    "\t-S\tShared library to be loaded\n"
    "\t-F\tFunction name in Shared library to be call after injection "
    "success (optional)\n"

    "\t-v\tPrint the version\n"
    "\t-V\tVerbose\n";

static const char opt_shorts[] = "hvVp:S:F:";

struct {
  char const* lib_path;
  char const* fn_name;

  pid_t pid;
  bool verbose;
} opt;

static pid_t parse_pid(const char* s) {
  char* e;
  long l = strtol(s, &e, 10);
  if (errno || *e != '\0' || l <= 0 || l > INT_MAX) pdie("Failed to parse -p option : %s", s);
  return (pid_t)l;
}

static void parse_opt(int argc, char** argv) {
  int c;
  while (0 < (c = getopt(argc, argv, opt_shorts))) {
    switch (c) {
      case 'h':
        puts(help_text);
        exit(EXIT_SUCCESS);
      case 'v':
        puts(version);
        exit(EXIT_SUCCESS);
      case 'V':
        opt.verbose = true;
        break;
      case 'p':
        opt.pid = parse_pid(optarg);
        break;
      case 'S':
        opt.lib_path = optarg;
        break;
      case 'F':
        opt.fn_name = optarg;
        break;
      default:
        fputs(help_text, stderr);
        exit(EXIT_FAILURE);
    }
  }

  bool invalid_opt = false;
  if (opt.pid <= 0) {
    fprintf(stderr, "Procce id can not <= 0\n");
    invalid_opt = true;
  }
  if (!opt.lib_path) {
    fprintf(stderr, "Need a lib path to inject\n");
    invalid_opt = true;
  }

  if (opt.verbose) {
    fprintf(stderr,

            "Proccess id : %d\n"
            "Shared library : %s\n"
            "Functiona : %s\n",
            opt.pid, opt.lib_path, opt.fn_name);
  }

  if (invalid_opt) pdie("Invalid options");
}

#include <inttypes.h>

#include "proc.h"

static void print_mem_region(FILE* f, ProcMemRegion* reg) {
  fprintf(f, "%" PRIxPTR "-%" PRIxPTR " %s %lx\n", reg->start, reg->end, reg->perm, reg->offset);
}

static void print_proc_map(FILE* f, ProcMap* map) {
  if (!map) return;
  fprintf(f, "Map Name %s\n", map->name);

  for (size_t idx = 0; idx < map->count; idx++) print_mem_region(f, map->regions + idx);
}

#include "dl_fn_get.h"
#include "inject.h"

int main(int argc, char** argv) {
  errno = 0;
  parse_opt(argc, argv);

  size_t buf_size = 0x200;
  char* line_buf = malloc(buf_size);

  FILE* maps_fp;
  {
    int pid = opt.pid;

    static const char proc_maps_fmt[] = "/proc/%d/maps";
    int n;
    n = snprintf(line_buf, buf_size, proc_maps_fmt, pid);

    if (n < 0) pdie("snprintf");
    while ((size_t)n >= buf_size) {
      buf_size = n + 1;
      line_buf = realloc(line_buf, buf_size);
      if (!line_buf) pdie(nullptr);
      n = snprintf(line_buf, buf_size, proc_maps_fmt, pid);
      if (n < 0) pdie("snprintf");
    }
    maps_fp = fopen(line_buf, "r");
    if (!maps_fp) pdie("Failed to open %s", line_buf);
  }

  static const char* lib_base_names[] = {"dl", "c", nullptr};

  for (char const **lib_ptr = lib_base_names, *lib; (lib = *lib_ptr); lib_ptr++) {
    (void)fseek(maps_fp, 0L, SEEK_SET);
    ProcMap* map = FindLibMapFromPid(maps_fp, lib, &line_buf, &buf_size);
    if (!map) continue;
    if (opt.verbose) print_proc_map(stderr, map);
    Dl_fn dl;
    if (!dl_fn_from(&dl, map->name)) goto skip;

    if (opt.verbose) printf("dlopen %" PRIxPTR " dlsym %" PRIxPTR "\n", dl.dlopen, dl.dlsym);

    if (opt.fn_name && !dl.dlsym) pdie("Not fount dlsym in %s", map->name);

    uintptr_t dlopen, dlsym;
    for (size_t i = 0; i < map->count; i++) {
      auto region = map->regions + i;

      if ((uintptr_t)region->offset <= dl.dlopen && dl.dlopen <= (region->end - region->start)) {
        dlopen = region->start + dl.dlopen - region->offset;
      };
      if ((uintptr_t)region->offset <= dl.dlsym && dl.dlsym <= (region->end - region->start)) {
        dlsym = region->start + dl.dlsym - region->offset;
      };
    }
    auto ret = inject(&line_buf, &buf_size, dlopen, dlsym, opt.lib_path, opt.fn_name, opt.pid);
    switch (ret) {
      case PL_SUCCESS:
        break;
      case PL_FDLOPEN:
        pdie("Failed to call dlopen in target");
      case PL_FDLSYM:
        pdie("Failed to call dlsym with fn in target");
      default:
        unreachable();
    }

    return EXIT_SUCCESS;

  skip:
    DropProcMap(map);
  }

  //  fclose(maps_fp);
  //  free(line_buf);
  pdie("Not found library held dlopen and dlsym obviously");
}
