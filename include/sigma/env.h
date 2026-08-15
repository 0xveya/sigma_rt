#pragma once

#include <allocator_vtable.h>
#include <qol.h>

SIGMA_SLICE(char, str_t);

typedef struct {
  str_t key;
  str_t value;
  bool occupied;
} map_entry_t;

typedef struct {
  map_entry_t *entries;
  usize len;
  usize cap;
  allocator_t alloc;
} str_map_t;

bool sigma_env_init(str_map_t *env, allocator_t alloc, char **envp);

void str_map_deinit(str_map_t *map);
str_t *str_map_get(str_map_t *map, str_t key);
bool str_map_has(str_map_t *map, str_t key);
bool str_map_init(str_map_t *map, allocator_t alloc, usize capacity);
bool str_map_put(str_map_t *map, str_t key, str_t value);
