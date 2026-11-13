#include <stddef.h>

[[noreturn]] void payload_exit(size_t _) {
  __asm__("int3");
  unreachable();
}
