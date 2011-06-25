// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"
#include "static_checking.h"

namespace circa {
namespace branch_members_function {

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

    CA_FUNCTION(branch_ref)
    {
        set_branch(OUTPUT, (INPUT_TERM(0)->nestedContents));
    }

    CA_FUNCTION(get_terms)
    {
        Branch* branch = as_branch(INPUT(0));
        if (branch == NULL)
            return error_occurred(CONTEXT, CALLER, "NULL branch");

        List& output = *List::cast(OUTPUT, branch->length());

        for (int i=0; i < branch->length(); i++)
            set_ref(output[i], branch->get(i));
    }

    CA_FUNCTION(get_term)
    {
        Branch* branch = as_branch(INPUT(0));
        if (branch == NULL)
            return error_occurred(CONTEXT, CALLER, "NULL branch");

        int index = INT_INPUT(1);
        set_ref(OUTPUT, branch->get(index));
    }

    CA_FUNCTION(format_source)
    {
        Branch* branch = as_branch(INPUT(0));
        if (branch == NULL)
            return error_occurred(CONTEXT, CALLER, "NULL branch");

        List* output = List::cast(OUTPUT, 0);
        format_branch_source((StyledSource*) output, *branch);
    }

    CA_FUNCTION(has_static_error)
    {
        Branch* branch = as_branch(INPUT(0));
        set_bool(OUTPUT, has_static_errors_cached(*branch));
    }

    CA_FUNCTION(get_static_errors)
    {
        Branch* branch = as_branch(INPUT(0));

        if (is_null(&branch->staticErrors))
            set_list(OUTPUT, 0);
        else
            copy(&branch->staticErrors, OUTPUT);
    }

    CA_FUNCTION(get_static_errors_formatted)
    {
        Branch* branch = as_branch(INPUT(0));

        if (is_null(&branch->staticErrors))
            set_list(OUTPUT, 0);

        List& list = *List::checkCast(&branch->staticErrors);
        List& out = *List::cast(OUTPUT, list.length());
        for (int i=0; i < list.length(); i++)
            format_static_error(list[i], out[i]);
    }

    CA_FUNCTION(evaluate)
    {
        Branch* branch = as_branch(INPUT(0));
        evaluate_branch_internal_with_state(CONTEXT, CALLER, *branch);
    }

    void setup(Branch& kernel)
    {
        import_function(kernel, branch_ref,
            "def branch_ref(any branch :ignore_error) -> Branch");

        import_member_function(kernel["Branch"], get_terms,
                "get_terms(Branch) -> List");
        import_member_function(kernel["Branch"], get_term,
                "get_term(Branch, int index) -> Ref");
        import_member_function(kernel["Branch"], format_source,
                "format_source(Branch) -> StyledSource");
        import_member_function(kernel["Branch"], has_static_error,
                "has_static_error(Branch) -> bool");
        import_member_function(kernel["Branch"], get_static_errors,
                "get_static_errors(Branch) -> List");
        import_member_function(kernel["Branch"], get_static_errors_formatted,
                "get_static_errors_formatted(Branch) -> List");
        import_member_function(kernel["Branch"], evaluate,
                "evaluate(Branch)");
    }
}
}
