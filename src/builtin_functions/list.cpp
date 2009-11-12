// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include "circa.h"

namespace circa {
namespace list_function {

    void evaluate(Term* caller) {
        Branch& dest = as_branch(caller);

        int numToAssign = std::min(caller->numInputs(), dest.length());

        for (int i=0; i < numToAssign; i++) {
            if (!is_value_alloced(caller->input(i)))
                continue;
            assign_value(caller->input(i), dest[i]);
        }

        // Add terms if necessary
        for (int i=dest.length(); i < caller->numInputs(); i++)
            create_duplicate(dest, caller->input(i));

        // Remove terms if necessary
        for (int i=caller->numInputs(); i < dest.length(); i++)
            dest[i] = NULL;

        dest.removeNulls();
    }

    std::string list_toSource(Term* caller) {
        std::stringstream out;
        prepend_name_binding(caller, out);
        out << "[";
        for (int i=0; i < caller->numInputs(); i++)
            out << get_source_of_input(caller, i);
        out << "]";
        return out.str();
    }

    void evaluate_repeat(Term* caller) {
        Branch& dest = as_branch(caller);

        Term* sourceTerm = caller->input(0);
        int repeatCount = int_input(caller, 1);
        int numToAssign = std::min(repeatCount, dest.length());

        for (int i=0; i < numToAssign; i++)
            assign_value(sourceTerm, dest[i]);

        // Add terms if necessary
        for (int i=dest.length(); i < repeatCount; i++)
            create_duplicate(dest, sourceTerm);

        // Remove terms if necessary
        for (int i=repeatCount; i < dest.length(); i++)
            dest[i] = NULL;

        dest.removeNulls();
    }

    void setup(Branch& kernel)
    {
        LIST_FUNC = import_function(kernel, evaluate, "list(any...) :: List");
        function_t::get_to_source_string(LIST_FUNC) = list_toSource;

        import_function(kernel, evaluate_repeat, "repeat(any, int) :: List");
    }
}
}
