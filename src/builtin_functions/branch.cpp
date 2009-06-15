// Copyright 2008 Andrew Fischer

#include "branch.h"
#include "circa.h"
#include "introspection.h"

namespace circa {
namespace branch_function {

    void evaluate(Term* caller)
    {
        Branch& branch = as_branch(caller);
        evaluate_branch(branch);
    }

    std::string toSourceString(Term* term) {

        if (term->boolPropOptional("syntaxHints:literal-list", false)) {
            std::stringstream out;
            prepend_name_binding(term, out);
            out << "[";
            Branch& contents = as_branch(term);
            out << get_branch_source(contents, "");
            out << "]";
            return out.str();
        } else {
            return get_term_source_default_formatting(term);
        }
    }

    void setup(Branch& kernel)
    {
        BRANCH_FUNC = import_function(kernel, evaluate, "branch() : Branch");
        function_get_to_source_string(BRANCH_FUNC) = toSourceString;
    }
}
}
