// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "common_headers.h"
#include "input_info.h"
#include "tagged_value.h"
#include "types/list.h"

namespace circa {

void
InputInfo::toTaggedValue(TaggedValue* value)
{
    List* list = make_list(value, 2);
    make_int(list->get(0), relativeScope);

    List* steps_list = make_list(list->get(1), nestedStepCount);

    for (int i=0; i < nestedStepCount; i++)
        make_int(steps_list->get(i), steps[i].index);
}

std::string
InputInfo::toShortString()
{
    std::stringstream out;
    out << relativeScope << ":";
    for (int i=0; i < nestedStepCount; i++) {
        if (i != 0) out << ",";
        out << steps[i].index;
    }
    return out.str();
}

} // namespace circa
