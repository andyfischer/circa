// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace for_function {

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

    void evaluate_discard(Term* caller)
    {
        Term* forTerm = caller->input(0);
        get_for_loop_discard_called(forTerm)->asBool() = true;
    }

    void setup(Branch& kernel)
    {
        FOR_FUNC = import_function(kernel, evaluate_for_loop, "for(List) : Code");
        function_t::get_to_source_string(FOR_FUNC) = toSourceString;

        DISCARD_FUNC = import_function(kernel, evaluate_discard, "discard(any)");
    }
}
} // namespace circa
