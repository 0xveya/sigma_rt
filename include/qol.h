#pragma once

#include <stddef.h>
#include <stdint.h>

#define var auto

typedef size_t usize;
typedef ptrdiff_t isize;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float f32;
typedef double f64;

typedef uintptr_t uptr;
typedef intptr_t iptr;

typedef long syscall_result;

#define SIGMA_SLICE(T, name)                                                   \
  typedef struct name {                                                        \
    usize len;                                                                 \
    T *items;                                                                  \
  } name

#define SYS_READ 0
#define SYS_WRITE 1
#define SYS_CLOSE 3

static inline syscall_result sigma_write(int fd, const void *buf, usize count) {
  syscall_result result;

  __asm__ volatile("syscall"
                   : "=a"(result)
                   : "a"(SYS_WRITE), "D"(fd), "S"(buf), "d"(count)
                   : "rcx", "r11", "memory");

  return result;
}
