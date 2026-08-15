#include <arena_allocator.h>
#include <sigma/rt.h>
#include <sigma/env.h>
#include <sigma_malloc.h>

#include <stdio.h>
#include <stdlib.h>

#define RT_ARENA_BLOCK_SIZE (16 * 1024)

typedef struct rt_allocators {
  sigma_allocator_t root;
  allocator_arena_t arena;
} rt_allocators_t;

static rt_allocators_t g_rt_allocators;

static _Noreturn void rt_panic(const char *message) {
  fprintf(stderr, "sigma_rt: %s\n", message);
  abort();
}

void sigma_rt_init(sigma_rt_t *rt, int argc, char **argv, char **envp) {
  if (rt == NULL)
    rt_panic("runtime context is null");

  const memory_source_t *source = &mmap_memory_source;
#ifdef SIGMA_MALLOC_BACKEND
  source = &malloc_memory_source;
#endif

  *rt = (sigma_rt_t){.args = sigma_args_from_raw(argc, argv)};
  sigma_allocator_init(&g_rt_allocators.root, source);
  rt->allocator = sigma_allocator(&g_rt_allocators.root);
  allocator_arena_init(&g_rt_allocators.arena, rt->allocator,
                       RT_ARENA_BLOCK_SIZE);
  rt->arena = allocator_arena(&g_rt_allocators.arena);

  if (!g_rt_allocators.root.initialized || rt->allocator.ctx == NULL ||
      rt->allocator.vtable == NULL || rt->arena.ctx == NULL ||
      rt->arena.vtable == NULL)
    rt_panic("sigma_malloc failed to initialize");

  if (!sigma_env_init(&rt->env, rt->arena, envp))
    rt_panic("failed to initialize environment");
}

int sigma_rt(sigma_rt_t *rt) {
  int status = sigma_main(rt);
  allocator_arena_deinit(&g_rt_allocators.arena);
  return status;
}
