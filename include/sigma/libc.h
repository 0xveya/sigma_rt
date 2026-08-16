#pragma once

#include <qol.h>

/* Replaceable libc boundary for sigma_rt. */
int sigma_rt_libc_memcmp(const void *left, const void *right, usize size);
void *sigma_rt_libc_memset(void *ptr, int value, usize size);
usize sigma_rt_libc_strlen(const char *s);
