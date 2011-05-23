// Copyright (c) Paul Hodge. See LICENSE file for license terms.

namespace circa {

struct Point : TaggedValue
{
    float getX();
    float getY();
    void set(float x, float y);
    
    static Point* checkCast(TaggedValue* tv);
    static Point* cast(TaggedValue* tv);
};

} // namespace circa
