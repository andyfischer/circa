// Copyright (c) Paul Hodge. See LICENSE file for license terms.

namespace circa {

struct Point : Value
{
    float getX();
    float getY();
    void set(float x, float y);
    
    static Point* checkCast(Value* tv);
    static Point* cast(Value* tv);
};

} // namespace circa
