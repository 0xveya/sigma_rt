#include <arena_allocator.h>
#include <sigma/env.h>
#include <sigma/rt.h>
#include <sigma_malloc.h>

#include <stdlib.h>

#define RT_ARENA_BLOCK_SIZE (16 * 1024)

typedef struct rt_allocators {
  sigma_allocator_t root;
  allocator_arena_t arena;
} rt_allocators_t;

static rt_allocators_t g_rt_allocators;

static _Noreturn void rt_panic(const char *message) {
  char buf[1024];

  const char prefix[] = "sigma_rt: ";
  usize pos = 0;

  for (usize i = 0; i < sizeof(prefix) - 1; i++)
    buf[pos++] = prefix[i];

  while (*message && pos < sizeof(buf) - 1)
    buf[pos++] = *message++;

  buf[pos++] = '\n';

  sigma_write(2, buf, pos);

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

void sigma_rt_deinit(sigma_rt_t *rt) {
  (void)rt;

#ifdef SIGMA_RT_DEBUG
  /* later:
   * sigma_malloc_validate();
   * sigma_malloc_report_leaks();
   * other stuff but malloc does stuff alr but this could be extra sigma stuff
   */
#endif

  allocator_arena_deinit(&g_rt_allocators.arena);
}

int sigma_rt(sigma_rt_t *rt) {
  int status = sigma_main(rt);

  sigma_rt_deinit(rt);

  return status;
}

int sigma_entry(int argc, char **argv, char **envp) {
  sigma_rt_t rt;

  sigma_rt_init(&rt, argc, argv, envp);

  return sigma_rt(&rt);
}
