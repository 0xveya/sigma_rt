#include <sigma/rt.h>

#include <stdio.h>
#include <string.h>

int sigma_main(sigma_rt_t *rt) {
  usize *lengths = allocator_array(rt->arena, usize, rt->args.len);

  if (lengths == NULL)
    return 1;

  usize sum = 0;
  for (usize i = 0; i < rt->args.len; i++) {
    lengths[i] = strlen(rt->args.items[i]);
    sum += lengths[i];
    printf("args[%zu] = %s\n", i, rt->args.items[i]);
  }
  for (usize i = 0; i < rt->env.len; i++) {
    map_entry_t *entry = &rt->env.entries[i];

    printf("%.*s = %.*s\n", (int)entry->key.len, entry->key.items,
           (int)entry->value.len, entry->value.items);
  }

  printf("argument length sum: %zu\n", sum);
  return 0;
}

int main(int argc, char **argv, char **envp) {
  sigma_rt_t rt;

  sigma_rt_init(&rt, argc, argv, envp);
  return sigma_rt(&rt);
}
