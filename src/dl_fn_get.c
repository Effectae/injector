#include "dl_fn_get.h"

#include <elf.h>
#include <fcntl.h>
#include <linux/elf.h>
#include <linux/mman.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "utils.h"

#ifdef __LP64__
typedef Elf64_Ehdr Elf_Ehdr;
typedef Elf64_Shdr Elf_Shdr;
typedef Elf64_Sym Elf_Sym;
typedef Elf64_Dyn Elf_Dyn;
#define ELFCLASS_NATIVE ELFCLASS64
#else
typedef Elf32_Ehdr Elf_Ehdr;
typedef Elf32_Shdr Elf_Shdr;
typedef Elf32_Sym Elf_Sym;
typedef Elf32_Dyn Elf_Dyn;
#define ELFCLASS_NATIVE ELFCLASS32
#endif

bool dl_fn_from(Dl_fn* dl, char const* elf_path) {
  int fd = open(elf_path, O_RDONLY);
  if (fd < 0) pdie("Failed to open %s", elf_path);

  struct stat statbuf;
  if (fstat(fd, &statbuf) < 0) pdie("Failed to get the stat of %s", elf_path);

  void* elf_m = mmap(nullptr, statbuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0L);
  if (elf_m == MAP_FAILED) pdie("Failed to mmap %s", elf_path);

  Elf_Ehdr const* elf_header = elf_m;
  if (memcmp(elf_header->e_ident, ELFMAG, SELFMAG) ||
      elf_header->e_ident[EI_CLASS] != ELFCLASS_NATIVE)
    goto failed_;

  Elf_Shdr const* shdr_header = elf_m + elf_header->e_shoff;

  // const char* shdr_strtable = elf_m + (shdr_header + elf_header->e_shstrndx)->sh_offset;

  Dl_fn d = {0, 0};

  for (size_t i = 0; i < elf_header->e_shnum; i++) {
    auto shdr_dynsym = shdr_header + i;
    if (shdr_dynsym->sh_type != SHT_DYNSYM) continue;

    char const* dynstr_table = shdr_header[shdr_dynsym->sh_link].sh_offset + elf_m;

    Elf_Sym const* sym_hd = elf_m + shdr_dynsym->sh_offset;

    for (size_t j = 0; j < shdr_dynsym->sh_size / sizeof(Elf_Sym); j++) {
      auto sym = sym_hd + j;
      char const* name = sym->st_name + dynstr_table;
      if (!strcmp(name, "dlopen")) {
        d.dlopen = sym->st_value;
        continue;
      }
      if (!strcmp(name, "dlsym")) d.dlsym = sym->st_value;
    }

    break;
  }
  if (!d.dlopen) goto failed_;
  *dl = d;

  if (munmap(elf_m, statbuf.st_size) < 0) pdie(nullptr);
  close(fd);
  return true;

failed_:

  if (munmap(elf_m, statbuf.st_size) < 0) pdie(nullptr);
  close(fd);
  return false;
}
