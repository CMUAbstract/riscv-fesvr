// See LICENSE for license details.

#ifndef _ELFLOADER_H
#define _ELFLOADER_H

#include <map>
#include <string>
#include <vector>

#include "elf.h"
#include "memif.h"

class memif_t;

typedef union {
	Elf32_Ehdr e32;
	Elf64_Ehdr e64;
} Elf_Ehdr;

typedef union {
	Elf32_Phdr e32;
	Elf64_Phdr e64;
} Elf_Phdr;

typedef union {
	Elf32_Shdr e32;
	Elf64_Shdr e64;
} Elf_Shdr;

typedef union {
	Elf32_Sym e32;
	Elf64_Sym e64;
} Elf_Sym;

class elfloader_t {
public:
	elfloader_t() : elfloader_t("", NULL, NULL) {}
	elfloader_t(std::string _fn, memif_t *_memif, reg_t *_entry);
	~elfloader_t() {}
	size_t load(std::string _fn, memif_t *_memif, reg_t *_entry);
	size_t load(void);
	bool is_loaded(void) { return loaded; }
	bool check_elf32(void) { return elf32; }
	Elf_Ehdr get_header() { return header; }
	std::map<std::string, Elf_Sym> get_symbols() { return symbols; }
	std::map<std::string, Elf_Shdr> get_sections() { return sections; }
	std::vector<Elf_Phdr> get_segments() { return segments; }
private:
	std::string fn;
	memif_t *memif;
	reg_t *entry;
	bool elf32 = false;
	bool loaded = false;
	bool info = false;
	Elf_Ehdr header;
	std::map<std::string, Elf_Sym> symbols;
	std::map<std::string, Elf_Shdr> sections;
	std::vector<Elf_Phdr> segments;
};

#endif
