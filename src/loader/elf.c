#include "elf.h"

#include "pmem.h"
#include "global_symtab.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct __attribute__((packed))
{
    unsigned char e_ident[16];
    uint16_t      e_type;
    uint16_t      e_machine;
    uint32_t      e_version;
    uint32_t      e_entry;
    uint32_t      e_phoff;
    uint32_t      e_shoff;
    uint32_t      e_flags;
    uint16_t      e_ehsize;
    uint16_t      e_phentsize;
    uint16_t      e_phnum;
    uint16_t      e_shentsize;
    uint16_t      e_shnum;
    uint16_t      e_shstrndx;
} Elf32Header;

typedef struct __attribute__((packed))
{
    uint32_t sh_name;
    uint32_t sh_type;
    uint32_t sh_flags;
    uint32_t sh_addr;
    uint32_t sh_offset;
    uint32_t sh_size;
    uint32_t sh_link;
    uint32_t sh_info;
    uint32_t sh_addralign;
    uint32_t sh_entsize;
} SectionHeader;

// sh_type values
#define SHT_NULL     0x0
#define SHT_PROGBITS 0x1
#define SHT_SYMTAB   0x2
#define SHT_STRTAB   0x3
#define SHT_RELA     0x4
#define SHT_HASH     0x5
#define SHT_DYNAMIC  0x6
#define SHT_NOTE     0x7
#define SHT_NOBITS   0x8
#define SHT_REL      0x9
#define SHT_SHLIB    0xa
#define SHT_DYNSYM   0xb

// sh_flags values
#define SHF_WRITE     0x1
#define SHF_ALLOC     0x2
#define SHF_EXECINSTR 0x4

typedef struct __attribute__((packed))
{
    uint32_t      st_name;
    uint32_t      st_value;
    uint32_t      st_size;
    unsigned char st_info;
    unsigned char st_other;
    uint16_t      st_shndx;
} SymtabEntry;

// accessors to 'bind' and 'type' from st_info
#define ST_BIND(i) ((i) >> 4)
#define ST_TYPE(i) ((i) & 0x0f)

// symbol bindings
#define STB_LOCAL  0x0
#define STB_GLOBAL 0x1
#define STB_WEAK   0x2

// symbol types
#define STT_NOTYPE  0x0
#define STT_OBJECT  0x1
#define STT_FUNC    0x2
#define STT_SECTION 0x3
#define STT_FILE    0x4

typedef struct __attribute__((packed))
{
    uint32_t r_offset;
    uint32_t r_info;
    int32_t  r_addend;
} RelaEntry;

// accessors to 'sym' and 'type' from r_info
#define R_SYM(i)  ((i) >> 8)
#define R_TYPE(i) ((unsigned char)i)

// relocation types
#define R_MSP430_16      0x3
#define R_MSP430_16_BYTE 0x5

static unsigned char magic[] = {'\x7f', 'E', 'L', 'F', '\x01', '\x01', '\x01'};

static SectionHeader* get_section_header(Elf32Header* eh, unsigned n)
{
    if (n > eh->e_shnum)
        return NULL;

    SectionHeader* headers = (SectionHeader*)((char*)eh + eh->e_shoff);
    return headers + n;
}

static SectionHeader* get_sh_begin(Elf32Header* eh)
{
    return get_section_header(eh, 0);
}

static SectionHeader* get_sh_end(Elf32Header* eh)
{
    return get_section_header(eh, eh->e_shnum);
}

static const char* get_section_name(Elf32Header* eh, SectionHeader* sh)
{
    SectionHeader* strtab_sh = get_section_header(eh, eh->e_shstrndx);
    char* strtab = (char*)eh + strtab_sh->sh_offset;
    return strtab + sh->sh_name;
}

static const char* get_symbol_name(Elf32Header* eh, SectionHeader* symtab_sh,
                                   SymtabEntry* sym)
{
    SectionHeader* strtab_sh = get_section_header(eh, symtab_sh->sh_link);
    char* strtab = (char*)eh + strtab_sh->sh_offset;
    return strtab + sym->st_name;
}

static int should_allocate(SectionHeader* sh)
{
    return (sh->sh_flags & SHF_ALLOC) && sh->sh_size > 0;
}

// loads all sections in memory and returns an array of the load addresses
// the sections that aren't loaded will have a load address of NULL
// the caller should free the returned array
static void** load_sections(Elf32Header* eh, ElfModule* em)
{
    void** addresses = malloc(eh->e_shnum * sizeof(void*));
    if (addresses == NULL)
        return NULL;

    // these are needed often
    SectionHeader* sh_begin = get_sh_begin(eh);
    SectionHeader* sh_end = get_sh_end(eh);

    // calculate the ammount of data and program memory to allocate
    size_t pmem_size = 0;
    size_t dmem_size = 0;

    SectionHeader* sh;
    for (sh = sh_begin; sh < sh_end; sh++)
    {
        if (should_allocate(sh))
        {
            if (sh->sh_flags & SHF_WRITE) // section should be writeable
                dmem_size += sh->sh_size;
            else
                pmem_size += sh->sh_size;
        }
    }

    if (pmem_size == 0)
    {
        free(addresses);
        puts("No program code to load");
        return NULL;
    }

    void* pmem_start = pmem_malloc(pmem_size);
    if (pmem_start == NULL)
    {
        free(addresses);
        puts("Not enough ROM");
        return NULL;
    }

    void* dmem_start = NULL;
    if (dmem_size > 0)
    {
        dmem_start = malloc(dmem_size);
        if (dmem_start == NULL)
        {
            free(addresses);
            pmem_free(pmem_start);
            puts("Not enough RAM");
            return NULL;
        }
    }

    printf("Allocated memory: ROM %uB @%p, RAM %uB @%p\n",
           pmem_size, pmem_start, dmem_size, dmem_start);

    char* pmem_next = pmem_start;
    char* dmem_next = dmem_start;

    for (sh = sh_begin; sh < sh_end; sh++)
    {
        ptrdiff_t index = sh - sh_begin;

        if (should_allocate(sh))
        {
            void* src = (char*)eh + sh->sh_offset;
            void* dest;

            if (sh->sh_flags & SHF_WRITE)
            {
                dest = dmem_next;
                dmem_next += sh->sh_size;
            }
            else
            {
                dest = pmem_next;
                pmem_next += sh->sh_size;
            }

            if (sh->sh_type != SHT_NOBITS)
            {
                // data or text section
                printf("Copying %luB of section %ld to %p\n",
                       sh->sh_size, index, dest);
                memcpy(dest, src, sh->sh_size);
            }
            else
            {
                // bss section
                printf("Initializing %luB of section %ld with 0's\n",
                       sh->sh_size, index);
                memset(dest, 0, sh->sh_size);
            }

            addresses[index] = dest;
        }
        else
            addresses[index] = NULL;
    }

    em->pmem = pmem_start;
    em->dmem = dmem_start;
    return addresses;
}

static int relocate_section(Elf32Header* eh, SectionHeader* rela_sh,
                            void** addresses)
{
    SectionHeader* sh = get_section_header(eh, rela_sh->sh_info);
    if (sh == NULL)
    {
        puts("Relocation refers to non-existing section");
        return 0;
    }

    SectionHeader* symtab_sh = get_section_header(eh, rela_sh->sh_link);
    if (symtab_sh == NULL)
    {
        puts("Relocation refers to non-existing symbol table");
        return 0;
    }

    printf("Relocating section %s\n", get_section_name(eh, sh));

    SymtabEntry* symtab = (SymtabEntry*)((char*)eh + symtab_sh->sh_offset);
    char* base_addr = addresses[sh - get_sh_begin(eh)];
    RelaEntry* entries = (RelaEntry*)((char*)eh + rela_sh->sh_offset);
    size_t nb_entries = rela_sh->sh_size / sizeof(RelaEntry);

    size_t i;
    for (i = 0; i < nb_entries; i++)
    {
        RelaEntry* re = &entries[i];
        SymtabEntry* sym = &symtab[R_SYM(re->r_info)];

        // address where to apply the relocation
        void* rel_addr = base_addr + re->r_offset;

        // address to write (symbol value)
        void* sym_addr;
        if (sym->st_shndx == 0)
        {
            // global symbol
            sym_addr =
                get_global_symbol_value(get_symbol_name(eh, symtab_sh, sym));
        }
        else
        {
            // local symbol
            sym_addr = (char*)addresses[sym->st_shndx] + // section address
                       sym->st_value + // symbol offset in section
                       re->r_addend; // addend
        }

        if (sym_addr == NULL)
        {
            printf("Undefined reference to symbol '%s'",
                   get_symbol_name(eh, symtab_sh, sym));
            return 0;
        }

        unsigned char rel_type = R_TYPE(re->r_info);

        if (rel_type == R_MSP430_16 || rel_type == R_MSP430_16_BYTE)
        {
            // I have no idea what the difference between these two is; they
            // both are absolute 16-bit relocations
            *(void**)rel_addr = sym_addr;
        }
        else
        {
            printf("Unkown relocation type %u\n", rel_type);
            return 0;
        }

        SectionHeader* ref_sh = get_section_header(eh, sym->st_shndx);
        printf("%p: ref to %s+%ld (%s+%lu) patched to %p\n",
               rel_addr, get_symbol_name(eh, symtab_sh, sym), re->r_addend,
               get_section_name(eh, ref_sh), sym->st_value, sym_addr);
    }
}

static int relocate_sections(Elf32Header* eh, void** addresses)
{
    SectionHeader* sh;
    for (sh = get_sh_begin(eh); sh < get_sh_end(eh); sh++)
    {
        if (sh->sh_type == SHT_RELA)
        {
            if (!relocate_section(eh, sh, addresses))
                return 0;
        }
        else if (sh->sh_type == SHT_REL)
        {
            puts("SHT_REL not supported (yet?)");
            return 0;
        }
    }
}

int elf_load(void* file, size_t size, ElfModule* em)
{
    Elf32Header* eh = file;


    // start with some sanity checks
    if (memcmp(eh->e_ident, magic, sizeof(magic)) != 0)
    {
        puts("Wrong magic");
        return 1;
    }

    // check if this is a relocatable file
    if (eh->e_type != 0x01)
    {
        puts("Not relocatable");
        return 1;
    }

    // check if this is a binary for the MSP430
    if (eh->e_machine != 0x69)
    {
        puts("Wrong architecture");
        return 1;
    }

    // I would have no idea what to do if the reported section header size does
    // not match the documented one...
    if (eh->e_shentsize != sizeof(SectionHeader))
    {
        puts("Weird section header size");
        return 1;
    }

    // check if there are sections
    if (eh->e_shoff == 0)
    {
        puts("No section headers");
        return 1;
    }

    unsigned i;
    for (i = 0; i < eh->e_shnum; i++)
    {
        SectionHeader* sh = get_section_header(eh, i);
        printf("Section %u, name %s, type %u\n",
               i, get_section_name(eh, sh), sh->sh_type);
    }

    puts("Loading sections...");
    void** addresses = load_sections(eh, em);
    if (addresses == NULL)
        return 1;

    relocate_sections(eh, addresses);
    free(addresses);
    return 0;
}

