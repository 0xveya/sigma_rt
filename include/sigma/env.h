#pragma once

#include <sigma/hash_map.h>

bool sigma_env_init(str_map_t *env, allocator_t alloc, char **envp);
