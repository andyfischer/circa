// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace switch_function {

    void evaluate_case(caStack* stack) { }
    void evaluate_default_case(caStack* stack) { }

    void switch_formatSource(caValue* source, Term* term)
    {
        format_name_binding(source, term);
        append_phrase(source, "switch ", term, sym_Keyword);
        format_source_for_input(source, term, 0);
        format_block_source(source, nested_contents(term), term);
    }

    void case_formatSource(caValue* source, Term* term)
    {
        append_phrase(source, "case ", term, sym_Keyword);
        format_source_for_input(source, term, 0);
        format_block_source(source, nested_contents(term), term);
    }

    void setup(Block* kernel)
    {
        FUNCS.switch_func = import_function(kernel, evaluate_switch, "switch(any input) -> any");
        as_function(FUNCS.switch_func)->formatSource = switch_formatSource;

        //FUNCS.case_func = import_function(kernel, evaluate_case, "case(any input)");
        //as_function(FUNCS.case_func)->formatSource = case_formatSource;

        FUNCS.default_case = import_function(kernel, evaluate_default_case, "default_case()");
    }

} // namespace switch_function
} // namespace circa
