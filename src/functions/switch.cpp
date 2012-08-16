// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace switch_function {

    CA_FUNCTION(evaluate_case) { }
    CA_FUNCTION(evaluate_default_case) { }

    void switch_formatSource(caValue* source, Term* term)
    {
        format_name_binding(source, term);
        append_phrase(source, "switch ", term, name_Keyword);
        format_source_for_input(source, term, 0);
        format_branch_source(source, nested_contents(term), term);
    }

    int switch_getOutputCount(Term* term)
    {
        Branch* contents = nested_contents(term);

        // check if term is still being initialized:
        if (contents->length() == 0)
            return 1;

        Branch* outerRebinds = contents->getFromEnd(0)->contents();
        return outerRebinds->length() + 1;
    }

    const char* switch_getOutputName(Term* term, int outputIndex)
    {
        Branch* contents = nested_contents(term);

        // check if term is still being initialized:
        if (contents->length() == 0)
            return "";

        Branch* outerRebinds = contents->getFromEnd(0)->contents();
        return outerRebinds->get(outputIndex - 1)->name.c_str();
    }
    Type* switch_getOutputType(Term* term, int outputIndex)
    {
        if (outputIndex == 0)
            return &VOID_T;

        Branch* contents = nested_contents(term);

        // check if term is still being initialized:
        if (contents->length() == 0)
            return &ANY_T;

        Branch* outerRebinds = contents->getFromEnd(0)->contents();
        return outerRebinds->get(outputIndex - 1)->type;
    }

    void case_formatSource(caValue* source, Term* term)
    {
        append_phrase(source, "case ", term, name_Keyword);
        format_source_for_input(source, term, 0);
        format_branch_source(source, nested_contents(term), term);
    }

    void setup(Branch* kernel)
    {
        FUNCS.switch_func = import_function(kernel, evaluate_switch, "switch(any input) -> any");
        as_function(FUNCS.switch_func)->formatSource = switch_formatSource;
        as_function(FUNCS.switch_func)->getOutputCount = switch_getOutputCount;
        as_function(FUNCS.switch_func)->getOutputName = switch_getOutputName;
        as_function(FUNCS.switch_func)->getOutputType = switch_getOutputType;

        //FUNCS.case_func = import_function(kernel, evaluate_case, "case(any input)");
        //as_function(FUNCS.case_func)->formatSource = case_formatSource;

        FUNCS.default_case = import_function(kernel, evaluate_default_case, "default_case()");
    }

} // namespace switch_function
} // namespace circa
