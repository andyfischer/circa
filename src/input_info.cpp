// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "common_headers.h"
#include "input_info.h"
#include "tagged_value.h"
#include "types/list.h"

namespace circa {

void
InputInfo::toTaggedValue(TaggedValue* value)
{
    List* list = make_list(value);
    list->resize(2);
    make_int(list->get(0), relativeScope);
    List* steps_list = make_list(list->get(1));
    steps_list->resize(nestedStepCount);
    for (int i=0; i < nestedStepCount; i++)
        make_int(steps_list->get(i), steps[i].index);
}

} // namespace circa
