// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"

namespace circa {
namespace branch_ref_function {

    // homeless function, not used yet:
    bool is_considered_config(Term* term)
    {
        if (term == NULL) return false;
        if (term->name == "") return false;
        if (!is_value(term)) return false;
        if (is_get_state(term)) return false;
        if (is_hidden(term)) return false;

        // ignore branch-based types
        //if (is_branch(term)) return false;
        if (is_type(term)) return false;

        return true;
    }

    Branch* deref(TaggedValue* val)
    {
        if (!is_list(val))
            return NULL;
        return (Branch*) as_opaque_pointer(val->getIndex(0));
    }

    CA_FUNCTION(branch_ref)
    {
        List& output = *List::cast(OUTPUT, 1);
        Branch* ptr = &(INPUT_TERM(0)->nestedContents);
        set_opaque_pointer(output[0], ptr);
    }

    CA_FUNCTION(get_terms)
    {
        Branch* branch = deref(INPUT(0));
        if (branch == NULL)
            return error_occurred(CONTEXT, CALLER, "NULL branch");

        List& output = *List::cast(OUTPUT, branch->length());

        for (int i=0; i < branch->length(); i++)
            set_ref(output[i], branch->get(i));
    }

    CA_FUNCTION(get_term)
    {
        Branch* branch = deref(INPUT(0));
        if (branch == NULL)
            return error_occurred(CONTEXT, CALLER, "NULL branch");

        int index = INT_INPUT(1);
        set_ref(OUTPUT, branch->get(index));
    }

    CA_FUNCTION(format_source)
    {
        Branch* branch = deref(INPUT(0));
        if (branch == NULL)
            return error_occurred(CONTEXT, CALLER, "NULL branch");

        List* output = List::cast(OUTPUT, 0);
        format_branch_source((StyledSource*) output, *branch);
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, branch_ref,
            "def branch_ref(any branch +ignore_error) -> BranchRef");

        import_member_function(kernel["BranchRef"], get_terms,
                "get_terms(BranchRef) -> List");
        import_member_function(kernel["BranchRef"], get_term,
                "get_term(BranchRef, int index) -> Ref");
        import_member_function(kernel["BranchRef"], format_source,
                "format_source(BranchRef) -> StyledSource");
    }
}
}
