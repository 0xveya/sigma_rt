#pragma once

#include <qol.h>

SIGMA_SLICE(char *, sigma_args_t);

sigma_args_t sigma_args_from_raw(int argc, char **argv);
