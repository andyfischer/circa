// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "evaluation.h"

namespace circa {
namespace eval_context_t {

    EvalContext* get(TaggedValue* value)
    {
        return ((EvalContext*) value->value_data.ptr);
    }

    void visitHeap(Type*, TaggedValue* value, Type::VisitHeapCallback callback, TaggedValue* visitContext)
    {
        EvalContext* context = get(value);
        TaggedValue relIdent;

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

