#pragma once

#include <sigma/args.h>
#include <sigma_malloc.h>

typedef struct sigma_rt {
  sigma_args_t args;
  sigma_allocator_t sigma_allocator;
  allocator_t allocator;
} sigma_rt_t;

void sigma_rt_init(sigma_rt_t *rt, int argc, char **argv);
int sigma_main(sigma_rt_t *rt);
int sigma_rt(sigma_rt_t *rt);
