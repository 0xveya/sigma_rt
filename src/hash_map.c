#include <sigma/hash_map.h>
#include <sigma/libc.h>

/* sigma:begin
name: str.from_cstr
provides: str.from_cstr
deps: str.len
externals:
kind: function
*/
str_t str_from_cstr(const char *s) {
  return (str_t){
      .items = (char *)s,
      .len = sigma_rt_libc_strlen(s),
  };
}
/* sigma:end */

/* sigma:begin
name: str.hash
provides: str.hash
deps:
externals:
kind: function
*/
u64 str_hash(str_t str) {
  u64 hash = 5381;

  for (usize i = 0; i < str.len; i++)
    hash = ((hash << 5) + hash) + (u8)str.items[i];

  return hash;
}
/* sigma:end */

/* sigma:begin
name: str.map
provides: str.map
deps: mem.alloc, mem.compare, mem.set, str.hash
externals:
kind: function
*/
static bool str_eq(str_t a, str_t b) {
  if (a.len != b.len)
    return false;

  return sigma_rt_libc_memcmp(a.items, b.items, a.len) == 0;
}

static map_entry_t *str_map_find_entry(map_entry_t *entries, usize cap,
                                       str_t key) {
  usize idx = str_hash(key) % cap;

  for (;;) {
    map_entry_t *entry = &entries[idx];

    if (!entry->occupied || str_eq(entry->key, key))
      return entry;

    idx = (idx + 1) % cap;
  }
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

  sigma_rt_libc_memset(map->entries, 0, capacity * sizeof(map_entry_t));
  map->cap = capacity;
  return true;
}

static bool str_map_grow(str_map_t *map) {
  usize new_cap = map->cap == 0 ? 8 : map->cap * 2;
  map_entry_t *new_entries = allocator_array(map->alloc, map_entry_t, new_cap);

  if (new_entries == NULL)
    return false;

  sigma_rt_libc_memset(new_entries, 0, new_cap * sizeof(map_entry_t));
  for (usize i = 0; i < map->cap; i++) {
    map_entry_t *old = &map->entries[i];

    if (old->occupied)
      *str_map_find_entry(new_entries, new_cap, old->key) = *old;
  }

  map->entries = new_entries;
  map->cap = new_cap;
  return true;
}

str_t *str_map_get(str_map_t *map, str_t key) {
  if (map == NULL || map->cap == 0)
    return NULL;

  usize idx = str_hash(key) % map->cap;
  for (usize tries = 0; tries < map->cap; tries++) {
    map_entry_t *entry = &map->entries[idx];

    if (!entry->occupied)
      return NULL;
    if (str_eq(entry->key, key))
      return &entry->value;
    idx = (idx + 1) % map->cap;
  }

  return NULL;
}

bool str_map_has(str_map_t *map, str_t key) {
  return str_map_get(map, key) != NULL;
}

bool str_map_put(str_map_t *map, str_t key, str_t value) {
  if (map == NULL)
    return false;

  if (map->cap != 0) {
    map_entry_t *entry = str_map_find_entry(map->entries, map->cap, key);

    if (entry->occupied) {
      entry->value = value;
      return true;
    }
  }

  if (map->cap == 0 || (map->len + 1) * 100 > map->cap * 67) {
    if (!str_map_grow(map))
      return false;
  }

  map_entry_t *entry = str_map_find_entry(map->entries, map->cap, key);
  entry->key = key;
  entry->value = value;
  entry->occupied = true;
  map->len++;
  return true;
}
/* sigma:end */
