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

    InputInfo()
      : relativeScope(0), nestedStepCount(0), steps(NULL)
    {}

    InputInfo(InputInfo const& copy)
    {
        relativeScope = copy.relativeScope;
        nestedStepCount = copy.nestedStepCount;
        size_t steps_size = copy.nestedStepCount*sizeof(InputInfo);
        steps = (NestedStep*) malloc(steps_size);
        memcpy(steps, copy.steps, steps_size);
    }

    ~InputInfo()
    {
        free(steps);
    }


    void setNestedStepCount(int count)
    {
        nestedStepCount = count;
        steps = (NestedStep*) realloc(steps, sizeof(NestedStep) * count);
    }
    void toTaggedValue(TaggedValue* value);
    std::string toShortString();
};

} // namespace circa
