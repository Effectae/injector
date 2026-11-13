#pragma once

#include "common_status.h"

typedef void* (*dlopen_t)(char const*, int);
typedef void* (*dlsym_t)(void*, char const*);

typedef void (*Fn)(void);

#include <stddef.h>

[[noreturn]]
void payload_exit(size_t exit_code);

#include <dlfcn.h>
