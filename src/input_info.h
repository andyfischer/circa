// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#pragma once

namespace circa {

struct InputInfo
{
    struct NestedStep {
        int index;
    };

    int relativeScope;

    int nestedStepCount;
    NestedStep *steps;

    InputInfo() : relativeScope(0), nestedStepCount(0), steps(NULL) {}

    ~InputInfo()
    {
        free(steps);
    }

    void setNestedStepCount(int count)
    {
        nestedStepCount = count;
        steps = (NestedStep*) realloc(steps, sizeof(NestedStep) * count);
    }
};

} // namespace circa
