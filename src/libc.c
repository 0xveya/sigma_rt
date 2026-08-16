#include <sigma/libc.h>

#include <string.h>

/* sigma:begin
name: libc.memcmp
provides: mem.compare
deps:
externals: memcmp
kind: provider
*/
int sigma_rt_libc_memcmp(const void *left, const void *right, usize size) {
  return memcmp(left, right, size);
}
/* sigma:end */

/* sigma:begin
name: libc.memset
provides: mem.set
deps:
externals: memset
kind: provider
*/
void *sigma_rt_libc_memset(void *ptr, int value, usize size) {
  return memset(ptr, value, size);
}
/* sigma:end */

/* sigma:begin
name: libc.strlen
provides: str.len
deps:
externals: strlen
kind: provider
*/
usize sigma_rt_libc_strlen(const char *s) { return strlen(s); }
/* sigma:end */
