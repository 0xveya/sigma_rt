#include <allocator_vtable.h>
#include <qol.h>
#include <sigma/env.h>

#include <string.h>

static inline usize get_env_len(char **envp) {
  usize len = 0;
  while (envp[len] != NULL)
    len++;
  return len;
}

static bool parse_env_entry(char *raw, str_t *key, str_t *value) {
  if (raw == NULL || key == NULL || value == NULL)
    return false;

  usize eq = 0;

  while (raw[eq] != '=' && raw[eq] != '\0')
    eq++;

  if (raw[eq] == '\0')
    return false;

  if (eq == 0)
    return false;

  *key = (str_t){
      .len = eq,
      .items = raw,
  };

  usize val_len = 0;

  while (raw[eq + 1 + val_len] != '\0')
    val_len++;

  *value = (str_t){
      .len = val_len,
      .items = raw + eq + 1,
  };

  return true;
}

bool sigma_env_init(str_map_t *env, allocator_t alloc, char **envp) {
  if (env == NULL || envp == NULL)
    return false;

  usize env_count = get_env_len(envp);

  if (!str_map_init(env, alloc, env_count))
    return false;

  for (usize i = 0; i < env_count; i++) {
    str_t key;
    str_t value;

    if (!parse_env_entry(envp[i], &key, &value))
      continue;

    if (!str_map_put(env, key, value))
      return false;
  }

  return true;
}

static bool str_eq(str_t a, str_t b) {
  if (a.len != b.len)
    return false;

  return memcmp(a.items, b.items, a.len) == 0;
}

bool str_map_init(str_map_t *map, allocator_t alloc, usize capacity) {
  if (map == NULL)
    return false;

  *map = (str_map_t){.alloc = alloc};
  if (capacity == 0)
    return true;

  map->entries = allocator_array(map->alloc, map_entry_t, capacity);
  if (map->entries == NULL)
    return false;

  map->cap = capacity;
  return true;
}

static bool str_map_grow(str_map_t *map) {
  if (map == NULL)
    return false;

  usize new_cap = map->cap == 0 ? 8 : map->cap * 2;
  map_entry_t *new_entries = allocator_array(map->alloc, map_entry_t, new_cap);

  if (new_entries == NULL)
    return false;

  if (map->entries != NULL && map->len > 0)
    memcpy(new_entries, map->entries, map->len * sizeof(map_entry_t));

  map->entries = new_entries;
  map->cap = new_cap;
  return true;
}

bool str_map_has(str_map_t *map, str_t key) {
  return str_map_get(map, key) != NULL;
}

str_t *str_map_get(str_map_t *map, str_t key) {
  if (map == NULL)
    return NULL;

  for (usize i = 0; i < map->len; i++) {
    if (str_eq(map->entries[i].key, key))
      return &map->entries[i].value;
  }

  return NULL;
}

bool str_map_put(str_map_t *map, str_t key, str_t value) {
  if (map == NULL)
    return false;

  for (usize i = 0; i < map->len; i++) {
    if (str_eq(map->entries[i].key, key)) {
      map->entries[i].value = value;
      return true;
    }
  }
  if ((map->cap == 0 || (map->len + 1) * 100 > map->cap * 67) &&
      !str_map_grow(map))
    return false;

  map->entries[map->len++] = (map_entry_t){
      .key = key,
      .value = value,
      .occupied = true,
  };

  return true;
}

str_t str_from_cstr(char *s) {
  return (str_t){
      .items = s,
      .len = strlen(s),
  };
}
