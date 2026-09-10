#include <sigma/rt.h>
#include <sigma/sys.h>

int sigma_main(sigma_rt_t *rt) {
  static const char message[] = "Hello, Sigma!\n";
  (void)rt;
  sigma_write_result_t result = s_write(1, message, sizeof(message) - 1);
  return result.ok ? 0 : 1;
}
