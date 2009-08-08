// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include "circa.h"

namespace circa {
namespace do_once_function {

    void evaluate(Term* caller)
    {
        bool &done = caller->input(0)->asBool();

        if (!done) {
            evaluate_branch(caller->asBranch());
            done = true;
        }
    }

    std::string toSourceString(Term* term)
    {
        std::stringstream result;
        result << "do once";
        result << term->stringPropOptional("syntaxHints:postHeadingWs", "\n");
        result << get_branch_source(as_branch(term));
        result << term->stringPropOptional("syntaxHints:preEndWs", "");
        result << "end";

        return result.str();
    }

    void setup(Branch& kernel)
    {
        DO_ONCE_FUNC = import_function(kernel, evaluate, "do_once(state bool) : Branch");
        function_t::get_to_source_string(DO_ONCE_FUNC) = toSourceString;
    }
}
}
