// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {
namespace if_function {

    void evaluate(Term* caller)
    {
        bool cond = as_bool(caller->input(0));

        if (cond) {
            Branch& branch = as_branch(caller);
            evaluate_branch(branch);
        }
    }

    std::string toSourceString(Term* term)
    {
        std::stringstream result;

        if (!term->boolPropOptional("if:is-else", false)) {
            result << "if";
            result << " " << get_source_of_input(term, 0) << std::endl;
        } else {
            result << "else";
        }

        result << get_branch_source(as_branch(term));

        if (!term->boolPropOptional("if:has-following-else", false))
            result << "end";

        return result.str();
    }

    void setup(Branch& kernel)
    {
        IF_FUNC = import_function(kernel, evaluate, "if(bool) : Branch");
        as_function(IF_FUNC).toSourceString = toSourceString;
    }
}
}
