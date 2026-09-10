#pragma once

#include <sigma/hash_map.h>

bool sigma_env_init(sigma_str_map *env, allocator_t alloc, char **envp);
