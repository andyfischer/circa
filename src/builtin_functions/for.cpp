// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace for_function {

    void evaluate(Term* caller)
    {
        evaluate_for_loop(caller, caller->input(0));
    }

    std::string toSourceString(Term* term)
    {
        std::stringstream result;
        result << "for ";
        result << get_for_loop_iterator(term)->name;
        result << " in ";
        result << get_source_of_input(term,0);
        result << term->stringPropOptional("syntaxHints:postHeadingWs", "\n");
        result << get_branch_source(as_branch(term));
        result << term->stringPropOptional("syntaxHints:preEndWs", "");
        result << "end";

        return result.str();
    }

    void setup(Branch& kernel)
    {
        FOR_FUNC = import_function(kernel, evaluate, "for(List) : Code");
        function_t::get_to_source_string(FOR_FUNC) = toSourceString;
    }
}
} // namespace circa
