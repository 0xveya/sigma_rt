#include <arena_allocator.h>
#include <sigma/hash_map.h>
#include <sigma/libc.h>
#include <sigma_malloc.h>

#include <stdio.h>

#define CHECK(condition)                                                       \
  do {                                                                         \
    if (!(condition)) {                                                        \
      fprintf(stderr, "%s:%d: check failed: %s\n", __FILE__, __LINE__,        \
              #condition);                                                     \
      return false;                                                            \
    }                                                                          \
  } while (0)

typedef struct {
  sigma_allocator_t root;
  allocator_arena_t arena;
  allocator_t alloc;
} test_allocator_t;

static bool test_allocator_init(test_allocator_t *test_alloc) {
  sigma_allocator_init(&test_alloc->root, &malloc_memory_source);
  test_alloc->alloc = sigma_allocator(&test_alloc->root);
  allocator_arena_init(&test_alloc->arena, test_alloc->alloc, 16 * 1024);
  test_alloc->alloc = allocator_arena(&test_alloc->arena);
  return test_alloc->alloc.ctx != NULL && test_alloc->alloc.vtable != NULL;
}

static bool str_is(str_t *value, const char *expected) {
  str_t expected_str = str_from_cstr(expected);
  return value != NULL && value->len == expected_str.len &&
         sigma_rt_libc_memcmp(value->items, expected_str.items,
                              expected_str.len) == 0;
}

static bool test_basic_insert_get_and_replace(allocator_t alloc) {
  str_map_t map;
  CHECK(str_map_init(&map, alloc, 4));
  CHECK(str_map_put(&map, str_from_cstr("A"), str_from_cstr("one")));
  CHECK(str_map_put(&map, str_from_cstr("B"), str_from_cstr("two")));
  CHECK(str_is(str_map_get(&map, str_from_cstr("A")), "one"));
  CHECK(str_is(str_map_get(&map, str_from_cstr("B")), "two"));
  CHECK(str_map_get(&map, str_from_cstr("MISSING")) == NULL);

  usize old_len = map.len;
  usize old_cap = map.cap;
  CHECK(str_map_put(&map, str_from_cstr("A"), str_from_cstr("changed")));
  CHECK(str_is(str_map_get(&map, str_from_cstr("A")), "changed"));
  CHECK(map.len == old_len);
  CHECK(map.cap == old_cap);
  return true;
}

static bool test_collisions(allocator_t alloc) {
  str_map_t map;
  CHECK(str_map_init(&map, alloc, 4));

  const char *left = "A";
  const char *right = "E";
  CHECK(str_hash(str_from_cstr(left)) % map.cap ==
        str_hash(str_from_cstr(right)) % map.cap);
  CHECK(str_map_put(&map, str_from_cstr(left), str_from_cstr("one")));
  CHECK(str_map_put(&map, str_from_cstr(right), str_from_cstr("five")));
  CHECK(str_is(str_map_get(&map, str_from_cstr(left)), "one"));
  CHECK(str_is(str_map_get(&map, str_from_cstr(right)), "five"));
  return true;
}

static bool test_growth_and_rehash(allocator_t alloc) {
  enum { KEY_COUNT = 100, KEY_SIZE = 16 };
  str_map_t map;
  char keys[KEY_COUNT][KEY_SIZE];
  char values[KEY_COUNT][KEY_SIZE];

  CHECK(str_map_init(&map, alloc, 1));
  for (usize i = 0; i < KEY_COUNT; i++) {
    CHECK(snprintf(keys[i], KEY_SIZE, "key-%zu", i) > 0);
    CHECK(snprintf(values[i], KEY_SIZE, "value-%zu", i) > 0);
    CHECK(str_map_put(&map, str_from_cstr(keys[i]), str_from_cstr(values[i])));
  }

  CHECK(map.len == KEY_COUNT);
  CHECK(map.cap >= KEY_COUNT);
  for (usize i = 0; i < KEY_COUNT; i++)
    CHECK(str_is(str_map_get(&map, str_from_cstr(keys[i])), values[i]));
  return true;
}

int main(void) {
  test_allocator_t test_alloc;
  if (!test_allocator_init(&test_alloc))
    return 1;

  bool passed = test_basic_insert_get_and_replace(test_alloc.alloc) &&
                test_collisions(test_alloc.alloc) &&
                test_growth_and_rehash(test_alloc.alloc);
  allocator_arena_deinit(&test_alloc.arena);
  return passed ? 0 : 1;
}
