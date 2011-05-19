// Copyright (c) Paul Hodge. See LICENSE file for license terms.

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

Point* Point::checkCast(Value* tv)
{
    if (!is_list(tv)) return NULL;
    if (tv->numElements() != 2) return NULL;
    if (!is_number(tv->getIndex(0))) return NULL;
    if (!is_number(tv->getIndex(1))) return NULL;
    return (Point*) tv;
}

Point*
Point::cast(Value* v)
{
    set_list(v, 2);
    set_float(v->getIndex(0), 0);
    set_float(v->getIndex(1), 0);
    return (Point*) v;
}

} // namespace circa
