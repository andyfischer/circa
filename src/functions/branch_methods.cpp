// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "circa.h"
#include "static_checking.h"

#include "types/ref.h"

namespace circa {
namespace branch_methods_function {

    CA_START_FUNCTIONS;

    CA_DEFINE_FUNCTION(branch_ref, "def branch_ref(any branch :ignore_error) -> Branch")
    {
        set_branch(OUTPUT, (INPUT_TERM(0)->nestedContents));
    }

    CA_DEFINE_FUNCTION(format_source, "Branch.format_source(self) -> StyledSource")
    {
        Branch* branch = as_branch(INPUT(0));
        if (branch == NULL)
            return error_occurred(CONTEXT, CALLER, "NULL branch");

        List* output = List::cast(OUTPUT, 0);
        format_branch_source((StyledSource*) output, *branch);
    }

    CA_DEFINE_FUNCTION(has_static_error, "Branch.has_static_error(self) -> bool")
    {
        Branch* branch = as_branch(INPUT(0));
        set_bool(OUTPUT, has_static_errors_cached(*branch));
    }

    CA_DEFINE_FUNCTION(get_static_errors, "Branch.get_static_errors(self) -> List")
    {
        Branch* branch = as_branch(INPUT(0));

        if (is_null(&branch->staticErrors))
            set_list(OUTPUT, 0);
        else
            copy(&branch->staticErrors, OUTPUT);
    }

    CA_DEFINE_FUNCTION(get_static_errors_formatted, "Branch.get_static_errors_formatted(self) -> List")
    {
        Branch* branch = as_branch(INPUT(0));

        if (is_null(&branch->staticErrors))
            set_list(OUTPUT, 0);

        List& list = *List::checkCast(&branch->staticErrors);
        List& out = *List::cast(OUTPUT, list.length());
        for (int i=0; i < list.length(); i++)
            format_static_error(list[i], out[i]);
    }

    CA_DEFINE_FUNCTION(evaluate, "Branch.evaluate()")
    {
        Branch* branch = as_branch(INPUT(0));
        evaluate_branch_internal_with_state(CONTEXT, CALLER, *branch);
    }

    // Reflection

    CA_DEFINE_FUNCTION(get_terms, "Branch.get_terms(self) -> List")
    {
        Branch* branch = as_branch(INPUT(0));
        if (branch == NULL)
            return error_occurred(CONTEXT, CALLER, "NULL branch");

        List& output = *List::cast(OUTPUT, branch->length());

        for (int i=0; i < branch->length(); i++)
            set_ref(output[i], branch->get(i));
    }

    CA_DEFINE_FUNCTION(get_term, "Branch.get_term(self, int index) -> Ref")
    {
        Branch* branch = as_branch(INPUT(0));
        if (branch == NULL)
            return error_occurred(CONTEXT, CALLER, "NULL branch");

        int index = INT_INPUT(1);
        set_ref(OUTPUT, branch->get(index));
    }

    bool is_considered_config(Term* term)
    {
        if (term == NULL) return false;
        if (term->name == "") return false;
        if (!is_value(term)) return false;
        if (is_get_state(term)) return false;
        if (is_hidden(term)) return false;
        if (is_function(term)) return false;

        // ignore branch-based types
        //if (is_branch(term)) return false;
        if (is_type(term)) return false;

        return true;
    }

    CA_DEFINE_FUNCTION(get_configs, "Branch.list_configs(self) -> List")
    {
        Branch* branch = as_branch(INPUT(0));
        if (branch == NULL)
            return error_occurred(CONTEXT, CALLER, "NULL branch");

        List& output = *List::cast(OUTPUT, 0);

        for (int i=0; i < branch->length(); i++) {
            Term* term = branch->get(i);
            if (is_considered_config(term))
                set_ref(output.append(), term);
        }
    }

    CA_DEFINE_FUNCTION(get_file_signature, "Branch.file_signature(self) -> any")
    {
        Branch* branch = as_branch(INPUT(0));
        if (branch == NULL)
            return error_occurred(CONTEXT, CALLER, "NULL branch");
        List* fileOrigin = branch_get_file_origin(branch);
        if (fileOrigin == NULL)
            set_null(OUTPUT);
        else
        {
            List* output = set_list(OUTPUT, 2);
            copy(fileOrigin->get(1), output->get(0));
            copy(fileOrigin->get(2), output->get(1));
        }
    }
    
    void setup(Branch& kernel)
    {
        CA_SETUP_FUNCTIONS(kernel);
    }
}
}
