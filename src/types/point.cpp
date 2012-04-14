// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "point.h"

namespace circa {

float Point::getX()
{
    return getIndex(0)->toFloat();
}

float Point::getY()
{
    return getIndex(1)->toFloat();
}

void Point::set(float x, float y)
{
    set_list(this, 2);
    set_float(getIndex(0), x);
    set_float(getIndex(1), y);
}

Point* Point::checkCast(caValue* tv)
{
    if (!is_list(tv)) return NULL;
    if (tv->numElements() != 2) return NULL;
    if (!is_number(tv->getIndex(0))) return NULL;
    if (!is_number(tv->getIndex(1))) return NULL;
    return (Point*) tv;
}

Point*
Point::cast(caValue* val)
{
    set_list(val, 2);
    set_float(val->getIndex(0), 0);
    set_float(val->getIndex(1), 0);
    return (Point*) val;
}

void set_point(caValue* val, float x, float y)
{
    set_list(val, 2);
    set_float(val->getIndex(0), x);
    set_float(val->getIndex(1), y);
}
void get_point(caValue* val, float* x, float* y)
{
    *x = to_float(get_index(val, 0));
    *y = to_float(get_index(val, 1));
}

} // namespace circa
