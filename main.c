#include <sigma/rt.h>

#include <stdio.h>

int sigma_main(sigma_rt_t *rt) {
  for (usize i = 0; i < rt->args.len; i++)
    printf("args[%zu] = %s\n", i, rt->args.items[i]);

  for (usize i = 0; i < rt->env.cap; i++) {
    const map_entry_t *entry = &rt->env.entries[i];

    if (!entry->occupied)
      continue;

    printf("%.*s = %.*s\n", (int)entry->key.len, entry->key.items,
           (int)entry->value.len, entry->value.items);
  }

  return 0;
}

int main(int argc, char **argv, char **envp) {
  sigma_rt_t rt;

  sigma_rt_init(&rt, argc, argv, envp);
  return sigma_rt(&rt);
}
