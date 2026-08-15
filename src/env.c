#include <qol.h>
#include <sigma/env.h>
#include <allocator_vtable.h>

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

  env->entries = allocator_array(alloc, map_entry_t, env_count);

  if (env_count != 0 && env->entries == NULL)
    return false;

  env->len = 0;
  env->cap = env_count;

  for (usize i = 0; i < env_count; i++) {
    str_t key;
    str_t value;

    if (!parse_env_entry(envp[i], &key, &value))
      continue;

    env->entries[env->len++] = (map_entry_t){
        .key = key,
        .value = value,
        .occupied = true,
    };
  }

  return true;
}
