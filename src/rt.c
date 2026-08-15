#include <sigma/rt.h>
#include <sigma_malloc.h>

#include <stdio.h>
#include <stdlib.h>

static _Noreturn void rt_panic(const char *message) {
  fprintf(stderr, "sigma_rt: %s\n", message);
  abort();
}

void sigma_rt_init(sigma_rt_t *rt, int argc, char **argv) {
  if (rt == NULL)
    rt_panic("runtime context is null");

  const memory_source_t *source = &mmap_memory_source;
#ifdef SIGMA_MALLOC_BACKEND
  source = &malloc_memory_source;
#endif

  *rt = (sigma_rt_t){.args = sigma_args_from_raw(argc, argv)};
  sigma_allocator_init(&rt->sigma_allocator, source);
  rt->allocator = sigma_allocator(&rt->sigma_allocator);

  if (!rt->sigma_allocator.initialized || rt->allocator.ctx == NULL ||
      rt->allocator.vtable == NULL)
    rt_panic("sigma_malloc failed to initialize");
}

int sigma_rt(sigma_rt_t *rt) { return sigma_main(rt); }
