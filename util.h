#ifndef UTIL_H
#define UTIL_H

#include <inttypes.h>
#include <string.h>
#include <limits>
#include <vector>

uint64_t pseudo_rng();
int do_hungarian(std::vector<std::vector<int>>);

#endif
