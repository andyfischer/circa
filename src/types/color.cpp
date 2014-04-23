// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {

void get_color(caValue* value, float* r, float* g, float* b, float* a)
{
    *r = to_float(get_index(value, 0));
    *g = to_float(get_index(value, 1));
    *b = to_float(get_index(value, 2));
    *a = to_float(get_index(value, 3));
}

} // namespace circa
