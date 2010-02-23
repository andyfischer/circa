// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include <circa.h>

namespace circa {
namespace for_function {

    std::string get_heading_source(Term* term)
    {
        std::stringstream result;
        result << "for ";
        result << get_for_loop_iterator(term)->name;
        result << " in ";
        result << get_source_of_input(term, 1);
        return result.str();
    }

    std::string toSourceString(Term* term)
    {
        std::stringstream result;
        result << get_heading_source(term);
        result << term->stringPropOptional("syntax:postHeadingWs", "\n");
        print_branch_source(result, term);
        result << term->stringPropOptional("syntax:whitespaceBeforeEnd", "");

        return result.str();
    }

    void evaluate_discard(EvalContext*, Term* caller)
    {
        Term* forTerm = caller->input(0);
        Term* discardCalled = get_for_loop_discard_called(forTerm);
        set_bool(discardCalled, true);
    }

    std::string discard_to_source_string(Term* term)
    {
        return "discard";
    }

    void setup(Branch& kernel)
    {
        FOR_FUNC = import_function(kernel, evaluate_for_loop, "for(List _state, List) -> Branch");
        function_t::get_to_source_string(FOR_FUNC) = toSourceString;
        function_t::set_input_meta(FOR_FUNC, 0, true); // allow _state to be NULL
        function_t::set_exposed_name_path(FOR_FUNC, "#rebinds_for_outer");

        DISCARD_FUNC = import_function(kernel, evaluate_discard, "discard(any)");
        function_t::get_to_source_string(DISCARD_FUNC) = discard_to_source_string;
        hide_from_docs(DISCARD_FUNC);
    }
}
} // namespace circa
