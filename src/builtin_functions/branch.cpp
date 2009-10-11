// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include "circa.h"

namespace circa {
namespace branch_function {

    void evaluate(Term* caller)
    {
        Branch& branch = as_branch(caller);
        evaluate_branch(branch, caller);
    }

    std::string toSourceString(Term* term) {

        std::stringstream out;

        if (term->boolPropOptional("syntaxHints:literal-list", false)) {
            prepend_name_binding(term, out);
            out << "[";
            Branch& contents = as_branch(term);
            out << get_branch_source(contents, "");
            out << "]";
            return out.str();
        } else if (term->type == NAMESPACE_TYPE) {
            out << "namespace ";
            out << term->name;
            out << term->stringPropOptional("syntaxHints:postHeadingWs", "\n");
            out << get_branch_source(as_branch(term));
            out << term->stringPropOptional("syntaxHints:preEndWs", "");
            out << "end";
            return out.str();
            
        } else {
            prepend_name_binding(term, out);
            out << "begin";
            out << term->stringPropOptional("syntaxHints:postHeadingWs", "\n");
            out << get_branch_source(as_branch(term));
            out << term->stringPropOptional("syntaxHints:preEndWs", "");
            out << "end";
            return out.str();
        }
    }

    void setup(Branch& kernel)
    {
        BRANCH_FUNC = import_function(kernel, evaluate, "branch() : Branch");
        function_t::get_to_source_string(BRANCH_FUNC) = toSourceString;
    }
}
}
