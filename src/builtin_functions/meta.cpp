// Copyright (c) 2007-2009 Paul Hodge. All rights reserved

#include <circa.h>

namespace circa {
namespace meta_function {

    Term* freeze_specializeType(Term* term)
    {
        return term->input(0)->type;
    }

    void recursively_evaluate_inputs_inside_branch(Term* term, Branch* branch)
    {
        if (term->owningBranch != branch)
            return;

        evaluate_term(term);

        for (int i=0; i < term->numInputs(); i++)
            recursively_evaluate_inputs_inside_branch(term->input(i), branch);
    }

    void lift_closure(Term* caller)
    {
        assign_value(caller->input(0), caller);

        Branch& contents = as_branch(caller);

        // TODO: Should do an evaluate_with_no_side_effects here (when it exists)

        for (int i=0; i < contents.length(); i++) {
            if (contents[i] == NULL) continue;
            if (contents[i]->function == FREEZE_FUNC) {
                Term* input = contents[i]->input(0);
                // This is flawed (see above comment)
                recursively_evaluate_inputs_inside_branch(input, contents[i]->owningBranch);

            }
        }
    }

    void setup(Branch& kernel)
    {
        FREEZE_FUNC = import_function(kernel, copy_function::evaluate, "freeze(any) : any");
        function_t::get_specialize_type(FREEZE_FUNC) = freeze_specializeType;

        Term* lc = import_function(kernel, lift_closure,
                "lift_closure(Branch) : Branch");
    }

} // namespace meta_function
} // namespace circa
