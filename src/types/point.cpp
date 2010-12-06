// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

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

Point* Point::checkCast(TaggedValue* tv)
{
    if (!is_list(tv)) return NULL;
    if (tv->numElements() != 2) return NULL;
    if (!is_float(tv->getIndex(0))) return NULL;
    if (!is_float(tv->getIndex(1))) return NULL;
    return (Point*) tv;
}

} // namespace circa
