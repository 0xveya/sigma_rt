#include <sigma/args.h>

sigma_args_t sigma_args_from_raw(int argc, char **argv) {
  return (sigma_args_t){
      .len = argc > 0 ? (usize)argc : 0,
      .items = argv,
  };
}
