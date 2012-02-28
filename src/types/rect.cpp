// Copyright (c) Paul Hodge. See LICENSE file for license terms.

namespace circa {

void get_rect(caValue* val, float* x1, float* y1, float* x2, float* y2)
{
    *x1 = to_float(get_index(val, 0));
    *y1 = to_float(get_index(val, 1));
    *x2 = to_float(get_index(val, 2));
    *y2 = to_float(get_index(val, 3));
}

} // namespace circa
