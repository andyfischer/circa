// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

#include "tinymt/tinymt64.h"

namespace circa {

struct RandState {
    TINYMT64_T tinymt;
};

void rand_init(RandState* state, int seed);
uint64_t rand_next_int(RandState* state);
double rand_next_double(RandState* state);

} // namespace circa
