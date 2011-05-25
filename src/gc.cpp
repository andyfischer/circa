// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "gc.h"
#include "tagged_value.h"
#include "type.h"

namespace circa {

void visit_heap(TaggedValue* value, Type::VisitHeapCallback callback,
        TaggedValue* context)
{
    Type::VisitHeap func = value->value_type->visitHeap;
    if (func != NULL)
        func(value->value_type, value, callback, context);
}

void dump_heap_callback(TaggedValue* value, TaggedValue* relativeIdentifier, TaggedValue* context)
{
    std::string prefix = as_string(context);

    std::cout << prefix << relativeIdentifier->toString()
       << " = " << value->toString() << std::endl;

    set_string(context, prefix + relativeIdentifier->toString() + ".");

    visit_heap(value, dump_heap_callback, context);

    set_string(context, prefix);
}

void recursive_dump_heap(TaggedValue* value, const char* prefix)
{
    TaggedValue context;
    set_string(&context, std::string(prefix) + ".");
    visit_heap(value, dump_heap_callback, &context);
}

void count_references_to_pointer_callback(TaggedValue* value, TaggedValue* relativeIdentifier, TaggedValue* context)
{
    if (is_opaque_pointer(value)) {
        List& contextList = *List::checkCast(context);
        if (as_opaque_pointer(value) == as_opaque_pointer(contextList[0]))
            set_int(contextList[1], as_int(contextList[1]) + 1);
    }

    visit_heap(value, count_references_to_pointer_callback, context);
}

int count_references_to_pointer(TaggedValue* container, void* ptr)
{
    List context;
    context.resize(2);
    set_opaque_pointer(context[0], ptr);
    set_int(context[1], 0);

    visit_heap(container, count_references_to_pointer_callback, &context);

    return as_int(context[1]);
}

} // namespace circa
