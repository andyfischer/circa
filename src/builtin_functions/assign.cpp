// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace assign_function {

    void evaluate(EvalContext* cxt, Term* caller)
    {
        Term* lexpr = caller->input(0);
        TaggedValue* value = caller->input(1);

        // Check for get index expression: a[i]
        if (lexpr->function == GET_INDEX_FUNC) {
            Term* root = lexpr->input(0);
            int index = lexpr->input(1)->asInt();
            copy(root, caller);
            copy(value, as_branch(caller)[index]);

        // Check for get field expression: a.f
        } else if (lexpr->function == GET_FIELD_FUNC) {
            Term* root = lexpr->input(0);
            std::string const& fieldName = lexpr->input(1)->asString();
            int index = root->value_type->findFieldIndex(fieldName);

            if (index == -1)
                return error_occurred(cxt, caller, "field not found: "+fieldName);

            copy(root, caller);
            copy(value, as_branch(caller)[index]);
        }
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, evaluate, "assign(any, any) -> any");
    }
}
}
