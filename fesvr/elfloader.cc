// See LICENSE for license details.
#include "elfloader.h"

#include <cstring>
#include <string>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <map>

elfloader_t::elfloader_t(std::string _fn, memif_t *_memif, reg_t *_entry) :
  fn(_fn), memif(_memif), entry(_entry) {
    int fd = open(fn.c_str(), O_RDONLY);
    if(fd != -1 && memif && entry) info = true;
}

size_t elfloader_t::load(std::string _fn, memif_t *_memif, reg_t *_entry) {
  fn = _fn;
  memif = _memif;
  entry = _entry;
  int fd = open(fn.c_str(), O_RDONLY);
  if(fd != -1 && memif && entry) info = true;
  return load();
}

size_t elfloader_t::load(void) {
  if(!info) return -1;
  int fd = open(fn.c_str(), O_RDONLY);
  struct stat s;
  assert(fd != -1);
  if (fstat(fd, &s) < 0)
    abort();
  size_t size = s.st_size;

  char* buf = (char*)mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
  assert(buf != MAP_FAILED);
  close(fd);

  assert(size >= sizeof(Elf64_Ehdr));
  const Elf64_Ehdr* eh64 = (const Elf64_Ehdr*)buf;
  assert(IS_ELF32(*eh64) || IS_ELF64(*eh64));

  std::vector<uint8_t> zeros;

  #define LOAD_ELF(ext, ehdr_t, phdr_t, shdr_t, sym_t) do { \
    ehdr_t* eh = (ehdr_t*)buf; \
    header.ext = *eh; \
    phdr_t* ph = (phdr_t*)(buf + eh->e_phoff); \
    *entry = eh->e_entry; \
    assert(size >= eh->e_phoff + eh->e_phnum*sizeof(*ph)); \
    for (unsigned i = 0; i < eh->e_phnum; i++) { \
      Elf_Phdr phdr = {.ext = ph[i]}; \
      segments.push_back(phdr); \
      if(ph[i].p_type == PT_LOAD && ph[i].p_memsz) { \
        if (ph[i].p_filesz) { \
          assert(size >= ph[i].p_offset + ph[i].p_filesz); \
          memif->write(ph[i].p_paddr, ph[i].p_filesz, (uint8_t*)buf + ph[i].p_offset); \
        } \
        zeros.resize(ph[i].p_memsz - ph[i].p_filesz); \
        memif->write(ph[i].p_paddr + ph[i].p_filesz, ph[i].p_memsz - ph[i].p_filesz, &zeros[0]); \
      } \
    } \
    shdr_t* sh = (shdr_t*)(buf + eh->e_shoff); \
    assert(size >= eh->e_shoff + eh->e_shnum*sizeof(*sh)); \
    assert(eh->e_shstrndx < eh->e_shnum); \
    assert(size >= sh[eh->e_shstrndx].sh_offset + sh[eh->e_shstrndx].sh_size); \
    char *shstrtab = buf + sh[eh->e_shstrndx].sh_offset; \
    unsigned strtabidx = 0, symtabidx = 0; \
    for (unsigned i = 0; i < eh->e_shnum; i++) { \
      unsigned max_len = sh[eh->e_shstrndx].sh_size - sh[i].sh_name; \
      assert(sh[i].sh_name < sh[eh->e_shstrndx].sh_size); \
      assert(strnlen(shstrtab + sh[i].sh_name, max_len) < max_len); \
      if (sh[i].sh_type & SHT_NOBITS) continue; \
      assert(size >= sh[i].sh_offset + sh[i].sh_size); \
      sections[shstrtab + sh[i].sh_name].ext =  sh[i]; \
      if (strcmp(shstrtab + sh[i].sh_name, ".strtab") == 0) \
        strtabidx = i; \
      if (strcmp(shstrtab + sh[i].sh_name, ".symtab") == 0) \
        symtabidx = i; \
    } \
    if (strtabidx && symtabidx) { \
      char* strtab = buf + sh[strtabidx].sh_offset; \
      sym_t* sym = (sym_t*)(buf + sh[symtabidx].sh_offset); \
      for (unsigned i = 0; i < sh[symtabidx].sh_size/sizeof(sym_t); i++) { \
        unsigned max_len = sh[strtabidx].sh_size - sym[i].st_name; \
        assert(sym[i].st_name < sh[strtabidx].sh_size); \
        assert(strnlen(strtab + sym[i].st_name, max_len) < max_len); \
        symbols[strtab + sym[i].st_name].ext = sym[i]; \
      } \
    } \
  } while(0)

  if (IS_ELF32(*eh64)) {
    elf32 = true;
    LOAD_ELF(e32, Elf32_Ehdr, Elf32_Phdr, Elf32_Shdr, Elf32_Sym);
  } else {
    LOAD_ELF(e64, Elf64_Ehdr, Elf64_Phdr, Elf64_Shdr, Elf64_Sym);
  }

  munmap(buf, size);
  loaded = true;
  return 0;
}
