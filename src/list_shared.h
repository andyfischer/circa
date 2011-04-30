// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "common_headers.h"
#include "tagged_value.h"

namespace circa {

struct ListData {
    int refCount;
    int count;
    int capacity;
    TaggedValue items[0];
    // items has size [capacity].
};

ListData* allocate_list(int num_elements);
void free_list(ListData* data);

TaggedValue* list_get_element(TaggedValue* value, int index);

} // namespace circa
