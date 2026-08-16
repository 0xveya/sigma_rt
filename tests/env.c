#include <sigma/rt.h>
#include <sigma/libc.h>

#include <stdio.h>

static bool env_is(sigma_rt_t *rt, const char *key, const char *expected) {
  str_t *value = str_map_get(&rt->env, str_from_cstr(key));
  str_t expected_str = str_from_cstr(expected);

  return value != NULL && value->len == expected_str.len &&
         sigma_rt_libc_memcmp(value->items, expected_str.items,
                              expected_str.len) == 0;
}

int sigma_main(sigma_rt_t *rt) {
  if (!env_is(rt, "HOME", "/tmp") || !env_is(rt, "USER", "veya") ||
      !env_is(rt, "EMPTY", "") || !env_is(rt, "THING", "a=b=c") ||
      rt->env.len != 4)
    return 1;

  str_t *home = str_map_get(&rt->env, str_from_cstr("HOME"));
  printf("HOME = %.*s\n", (int)home->len, home->items);
  return 0;
}

int main(int argc, char **argv, char **envp) {
  sigma_rt_t rt;

  sigma_rt_init(&rt, argc, argv, envp);
  return sigma_rt(&rt);
}
