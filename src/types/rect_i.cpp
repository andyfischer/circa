// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "rect_i.h"

namespace circa {

void Rect_i::set(int x1, int y1, int x2, int y2)
{
    set_int(get_index(this, 0), x1);
    set_int(get_index(this, 1), y1);
    set_int(get_index(this, 2), x2);
    set_int(get_index(this, 3), y2);
}

Rect_i* Rect_i::cast(TaggedValue* tv)
{
    change_type(tv, unbox_type(RECT_I_TYPE_TERM));
    return (Rect_i*) tv;
}


} // namespace circa
