#include "common.h"

__attribute__((section(".text.start"))) void _start(dlopen_t dlopen, dlsym_t dlsym, char const* lib,
                                                    char const* fn) {
  auto hd = dlopen(lib, RTLD_LOCAL | RTLD_NOW);
  if (!hd) payload_exit(PL_FDLOPEN);
  auto f = dlsym(hd, fn);
  if (!f) payload_exit(PL_FDLSYM);
  ((Fn)f)();
  payload_exit(PL_SUCCESS);
}
