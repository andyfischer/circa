// Copyright (c) Paul Hodge. See LICENSE file for license terms.

namespace circa {
namespace eval_context_t {

    void visitHeap(Type*, Value* value, Type::VisitHeapCallback callback, void* userdata)
    {
        // TODO
    }

    void setup_type(Type* type)
    {
        type->name = "EvalContext";
        type->visitHeap = visitHeap;
    }
}
} // namespace circa

