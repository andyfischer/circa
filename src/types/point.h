// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#pragma once

namespace circa {

struct Point : TValue
{
    float getX();
    float getY();
    void set(float x, float y);
    
    static Point* checkCast(TValue* tv);
    static Point* cast(TValue* tv);
};

void set_point(TValue* val, float x, float y);
void get_point(TValue* val, float* x, float* y);

} // namespace circa
