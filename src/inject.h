#pragma once

#include <stdint.h>
#include <sys/types.h>

#include "../payload/common_status.h"

PayloadExitCode inject(char **buf,size_t *buf_size,uintptr_t dlopen, uintptr_t dlsym, char const* lib, char const* fn,
                       pid_t pid);
