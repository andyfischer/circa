// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace eval_context_t {

    Stack* get(caValue* value)
    {
        return ((Stack*) value->value_data.ptr);
    }

    void visitHeap(Type*, caValue* value, Type::VisitHeapCallback callback, caValue* visitContext)
    {
        Stack* context = get(value);
        Value relIdent;

        set_string(&relIdent, "state");
        callback(&context->state, &relIdent, visitContext);
    }

    void setup_type(Type* type)
    {
        set_string(&type->name, "Stack");
        type->visitHeap = visitHeap;
    }
}
} // namespace circa

