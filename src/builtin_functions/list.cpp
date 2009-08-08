// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {
namespace list_function {

    void evaluate(Term* caller) {
        Branch& dest = as_branch(caller);

        int lengthToAssign = std::min(caller->numInputs(), dest.length());

        for (int i=0; i < lengthToAssign; i++) {
            if (!is_value_alloced(caller->input(i)))
                continue;
            assign_value(caller->input(i), dest[i]);
        }

        // Add terms if necessary
        for (int i=dest.length(); i < caller->numInputs(); i++) {
            create_duplicate(dest, caller->input(i));
        }

        // Remove terms if necessary
        for (int i=caller->numInputs(); i < dest.length(); i++) {
            dest[i] = NULL;
        }
        dest.removeNulls();
    }

    std::string toSourceString(Term* caller) {
        std::stringstream out;
        if (caller->name != "")
            out << caller->name << " = ";
        out << "[";
        for (int i=0; i < caller->numInputs(); i++)
            out << get_source_of_input(caller, i);
        out << "]";
        return out.str();
    }

    void setup(Branch& kernel)
    {
        LIST_FUNC = import_function(kernel, evaluate, "list(any...) : List");
        function_t::get_to_source_string(LIST_FUNC) = toSourceString;
    }
}
}
