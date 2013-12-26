// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "rand.h"

namespace circa {

void rand_init(RandState* state, int seed)
{
    tinymt64_init(&state->tinymt, seed);
}

uint64_t rand_next_int(RandState* state)
{
    return tinymt64_generate_uint64(&state->tinymt);
}

double rand_next_double(RandState* state)
{
    return tinymt64_generate_double(&state->tinymt);
}

};
