#include "inject.h"

#include <fcntl.h>
#include <linux/elf.h>
#include <linux/ptrace.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/uio.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>

#include "payload.h"
#include "utils.h"

PayloadExitCode inject(char** buf, size_t* buf_size, uintptr_t dlopen, uintptr_t dlsym,
                       char const* lib, char const* fn, pid_t pid) {
  (void)lib;
  (void)fn;
  (void)dlsym;
  (void)dlopen;

  int n;
  static const char proc_mem_fmt[] = "/proc/%d/mem";
  n = snprintf(*buf, *buf_size, proc_mem_fmt, pid);
  if (n < 0) pdie("snprintf");
  while ((size_t)n >= *buf_size) {
    *buf = realloc(*buf, *buf_size = n + 1);
    if (!*buf) pdie(nullptr);

    n = snprintf(*buf, *buf_size, proc_mem_fmt, pid);
    if (n < 0) pdie("snprintf");
  }

  int mem_fd = open(*buf, O_RDWR);
  if (mem_fd < 0) pdie("Failed to open %s", *buf);

  if (ptrace(PTRACE_ATTACH, pid, nullptr, nullptr) < 0) pdie("ATTACH");
  int status;
  if (waitpid(pid, &status, 0) < 0) pdie("waitpid");
  if (WIFEXITED(status) || WIFSIGNALED(status)) goto failed_;

  if (ptrace(PTRACE_SETOPTIONS, pid, nullptr, PTRACE_O_TRACESYSGOOD) < 0)
    pdie("Failed to set tracesysgood");
  if (ptrace(PTRACE_SYSCALL, pid, nullptr, nullptr) < 0) pdie("PTRACE SYSCALL");

  while (true) {
    if (waitpid(pid, &status, 0) < 0) pdie("waitpid");
    if (WIFEXITED(status) || WIFSIGNALED(status)) goto failed_;
    if (WIFSTOPPED(status)) {
      int sig = WSTOPSIG(status);
      if (sig != (SIGTRAP | 0x80)) {
        if (ptrace(PTRACE_SYSCALL, pid, 0, sig) < 0) pdie("PTRACE SYSCALL");
        continue;
      }

      struct user_regs_struct regs;
      struct iovec iov = {.iov_base = &regs, .iov_len = sizeof(regs)};
      if (ptrace(PTRACE_GETREGSET, pid, NT_PRSTATUS, &iov) < 0) pdie("Get register");

      auto lib_len = strlen(lib) + 1;
      auto fn_len = strlen(fn) + 1;
      {
        auto need_size = lib_len + fn_len + payload_size;
        if (*buf_size < need_size) *buf = realloc(*buf, need_size);
        char* b = *buf;
        memcpy(b, payload, payload_size);
        memcpy(b += payload_size, lib, lib_len);
        memcpy(b + lib_len, fn, fn_len);
      }

      if (ptrace(PTRACE_SETREGSET, pid, NT_PRSTATUS, &iov) < 0) pdie("Set register");
    }
  }

  if (ptrace(PTRACE_DETACH, pid, nullptr, nullptr) < 0) pdie("DETTACH");

  close(mem_fd);

  return PL_SUCCESS;

failed_:

  pdie("Failed to inject");
}
