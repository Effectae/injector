#include <stddef.h>

[[noreturn]] void payload_exit(size_t _) {
  __asm__("ebreak");
  unreachable();
}
