#pragma once

#include <allocator_vtable.h>
#include <sigma/args.h>

typedef struct sigma_rt {
  sigma_args_t args;
  allocator_t allocator;
  allocator_t arena;
} sigma_rt_t;

int sigma_main(sigma_rt_t *rt);
int sigma_rt(sigma_rt_t *rt);
void sigma_rt_init(sigma_rt_t *rt, int argc, char **argv);
