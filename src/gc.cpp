// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "gc.h"
#include "tagged_value.h"
#include "type.h"

namespace circa {

void visit_heap(TaggedValue* value, Type::VisitHeapCallback callback, void* userdata)
{
    Type::VisitHeap func = value->value_type->visitHeap;
    if (func != NULL)
        func(value->value_type, value, callback, userdata);
}

struct DumpHeapContext
{
    std::string prefix;
};

void dump_heap_callback(void* userdata, TaggedValue* value, TaggedValue* relativeIdentifier)
{
    DumpHeapContext* context = (DumpHeapContext*) userdata;

    std::cout << context->prefix << relativeIdentifier->toString()
       << " = " << value->toString() << std::endl;

    std::string oldprefix = context->prefix;

    context->prefix += relativeIdentifier->toString() + ".";

    visit_heap(value, dump_heap_callback, userdata);

    context->prefix = oldprefix;
}

void recursive_dump_heap(TaggedValue* value, const char* prefix)
{
    DumpHeapContext context;
    context.prefix = prefix;
    context.prefix += ".";
    visit_heap(value, dump_heap_callback, &context);
}

} // namespace circa
