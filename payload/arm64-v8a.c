#include <stddef.h>

[[noreturn]]
void payload_exit(size_t _) {
  __asm__("brk #0");
  unreachable();
}
