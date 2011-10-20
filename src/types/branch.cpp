// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "builtins.h"

namespace circa {
namespace branch_t {

    void visitHeap(Type*, TaggedValue* value, Type::VisitHeapCallback callback, TaggedValue* context)
    {
        if (as_branch(value) == NULL)
            return;

        Branch& branch = *as_branch(value);
        TaggedValue relIdent;

        set_string(&relIdent, "locals");
        callback(&branch.locals, &relIdent, context);

        set_string(&relIdent, "localsStack");
        callback(&branch.localsStack, &relIdent, context);

        set_string(&relIdent, "origin");
        callback(&branch.origin, &relIdent, context);

        set_string(&relIdent, "hasInlinedState");
        callback(&branch.hasInlinedState, &relIdent, context);

        set_string(&relIdent, "staticErrors");
        callback(&branch.staticErrors, &relIdent, context);

        set_string(&relIdent, "pendingUpdates");
        callback(&branch.pendingUpdates, &relIdent, context);

        for (int i=0; i < branch.length(); i++) {
            Term* term = branch[i];
            std::stringstream ident;
            ident << "term[" << i << "]";
            set_string(&relIdent, ident.str());
            callback(term, &relIdent, context);

            ident << ".nestedContents";
            set_string(&relIdent, ident.str());
    
            TaggedValue nestedContents;
            set_transient_value(&nestedContents, &term->nestedContents, &BRANCH_T);
            callback(&nestedContents, &relIdent, context);
            cleanup_transient_value(&nestedContents);
        }
    }

    void setup_type(Type* type)
    {
        type->name = "Branch";
        type->visitHeap = visitHeap;
    }
}
} // namespace circa
