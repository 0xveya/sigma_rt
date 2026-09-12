#include <sigma/sys.h>
#include <sigma/tls.h>

#include <stddef.h>
#include <stdint.h>

#if !defined(__linux__) || !defined(__x86_64__)
#error "sigma TLS currently supports Linux x86-64 only"
#endif

enum {
  SIGMA_AT_NULL = 0,
  SIGMA_AT_PHDR = 3,
  SIGMA_AT_PHNUM = 5,
  SIGMA_PT_TLS = 7,
};

typedef struct sigma_aux {
  uintptr_t type;
  uintptr_t value;
} sigma_aux_t;

typedef struct sigma_elf64_phdr {
  uint32_t type;
  uint32_t flags;
  uint64_t offset;
  uint64_t virtual_address;
  uint64_t physical_address;
  uint64_t file_size;
  uint64_t memory_size;
  uint64_t alignment;
} sigma_elf64_phdr_t;

static uintptr_t align_up(uintptr_t value, uintptr_t alignment) {
  return (value + alignment - 1) & ~(alignment - 1);
}

bool sigma_tls_init(char **envp) {
  if (envp == NULL)
    return false;

  while (*envp != NULL)
    envp++;

  const sigma_aux_t *aux = (const sigma_aux_t *)(envp + 1);
  const sigma_elf64_phdr_t *headers = NULL;
  size_t header_count = 0;

  for (; aux->type != SIGMA_AT_NULL; aux++) {
    if (aux->type == SIGMA_AT_PHDR)
      headers = (const sigma_elf64_phdr_t *)aux->value;
    else if (aux->type == SIGMA_AT_PHNUM)
      header_count = (size_t)aux->value;
  }

  if (headers == NULL || header_count == 0)
    return false;

  const sigma_elf64_phdr_t *tls = NULL;
  for (size_t i = 0; i < header_count; i++) {
    if (headers[i].type == SIGMA_PT_TLS) {
      tls = &headers[i];
      break;
    }
  }

  if (tls == NULL)
    return true;

  uintptr_t alignment = tls->alignment == 0 ? sizeof(void *) : tls->alignment;
  uintptr_t tls_size = align_up((uintptr_t)tls->memory_size, alignment);
  uintptr_t mapping_size = tls_size + alignment + sizeof(void *);
  sigma_mmap_result_t mapping =
      s_mmap(NULL, mapping_size, SIGMA_PROT_READ | SIGMA_PROT_WRITE,
             SIGMA_MAP_PRIVATE | SIGMA_MAP_ANONYMOUS, -1, 0);
  if (!mapping.ok)
    return false;

  uintptr_t thread_pointer =
      align_up((uintptr_t)mapping.value + tls_size, alignment);
  unsigned char *tls_block = (unsigned char *)(thread_pointer - tls_size);
  const unsigned char *initial =
      (const unsigned char *)(uintptr_t)tls->virtual_address;

  for (size_t i = 0; i < (size_t)tls->file_size; i++)
    tls_block[i] = initial[i];
  for (size_t i = (size_t)tls->file_size; i < (size_t)tls->memory_size; i++)
    tls_block[i] = 0;

  *(void **)thread_pointer = (void *)thread_pointer;
  sigma_status_result_t result =
      s_arch_prctl(SIGMA_ARCH_SET_FS, thread_pointer);
  return result.ok;
}
