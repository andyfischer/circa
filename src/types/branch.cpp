// Copyright (c) Paul Hodge. See LICENSE file for license terms.

namespace circa {
namespace branch_t {

    void visitHeap(Type*, TaggedValue* value, Type::VisitHeapCallback callback, TaggedValue* context)
    {
        // TODO
    }

    void setup_type(Type* type)
    {
        type->name = "Branch";
        type->visitHeap = visitHeap;
    }
}
} // namespace circa
