// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "evaluation.h"

namespace circa {
namespace eval_context_t {

    EvalContext* get(TValue* value)
    {
        return ((EvalContext*) value->value_data.ptr);
    }

    void visitHeap(Type*, TValue* value, Type::VisitHeapCallback callback, TValue* visitContext)
    {
        EvalContext* context = get(value);
        TValue relIdent;

        set_string(&relIdent, "state");
        callback(&context->state, &relIdent, visitContext);

        set_string(&relIdent, "messages");
        callback(&context->messages, &relIdent, visitContext);
    }

    void setup_type(Type* type)
    {
        type->name = "EvalContext";
        type->visitHeap = visitHeap;
    }
}
} // namespace circa

