// Copyright (c) Paul Hodge. See LICENSE file for license terms.

namespace circa {
namespace branch_t {

    Branch& get(TaggedValue* value)
    {
        return *((Branch*) value->value_data.ptr);
    }

    void visitHeap(Type*, TaggedValue* value, Type::VisitHeapCallback callback, TaggedValue* context)
    {
        Branch& branch = get(value);
        TaggedValue relIdent;

        set_string(&relIdent, "locals");
        callback(&branch.locals, &relIdent, context);

        set_string(&relIdent, "localsStack");
        callback(&branch.localsStack, &relIdent, context);

        set_string(&relIdent, "fileSignature");
        callback(&branch.fileSignature, &relIdent, context);

        set_string(&relIdent, "hasInlinedState");
        callback(&branch.hasInlinedState, &relIdent, context);

        set_string(&relIdent, "staticErrors");
        callback(&branch.staticErrors, &relIdent, context);

        set_string(&relIdent, "pendingUpdates");
        callback(&branch.pendingUpdates, &relIdent, context);
    }

    void setup_type(Type* type)
    {
        type->name = "Branch";
        type->visitHeap = visitHeap;
    }
}
} // namespace circa
