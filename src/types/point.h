// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

struct Point : caValue
{
    float getX();
    float getY();
    void set(float x, float y);
    
    static Point* checkCast(caValue* tv);
    static Point* cast(caValue* tv);
};

void set_point(caValue* val, float x, float y);
void get_point(caValue* val, float* x, float* y);

} // namespace circa
