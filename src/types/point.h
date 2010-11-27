// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

namespace circa {

struct Point : TaggedValue
{
    float getX();
    float getY();
    void set(float x, float y);
    
    static Point* checkCast(TaggedValue* tv);
};

} // namespace circa
