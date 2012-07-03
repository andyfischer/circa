// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

namespace circa {
namespace for_function {

    void format_heading(caValue* source, Term* term)
    {
        format_name_binding(source, term);
        append_phrase(source, "for ", term, name_Keyword);
        std::string explicitTypeName = term->stringProp("syntax:explicitType", "");
        if (explicitTypeName != "") {
            append_phrase(source, explicitTypeName, term, name_None);
            append_phrase(source, " ", term, name_Whitespace);
        }
        append_phrase(source, for_loop_get_iterator_name(term), term, name_None);
        append_phrase(source, " in ", term, name_Keyword);
        if (term->boolProp("modifyList", false))
            append_phrase(source, "@", term, name_None);
        format_source_for_input(source, term, 0);
    }

    void formatSource(caValue* source, Term* term)
    {
        format_heading(source, term);
        format_branch_source(source, nested_contents(term), term);
        append_phrase(source, term->stringProp("syntax:whitespaceBeforeEnd", ""),
            term, tok_Whitespace);
    }

    void setup(Branch* kernel)
    {
        FUNCS.for_func = import_function(kernel, NULL, "for(Indexable) -> List");
        as_function(FUNCS.for_func)->formatSource = formatSource;

        FUNCS.loop_iterator = import_function(kernel, NULL,
            "loop_iterator(any, any) -> int");
        FUNCS.loop_index = import_function(kernel, NULL, "loop_index(int index) -> int");
        function_set_empty_evaluation(as_function(FUNCS.loop_index));

        FUNCS.loop_output_index = import_function(kernel, NULL, "loop_output_index() -> any");
        function_set_empty_evaluation(as_function(FUNCS.loop_output_index));

        FUNCS.unbounded_loop = import_function(kernel, evaluate_unbounded_loop,
            "loop(bool condition)");
        FUNCS.unbounded_loop_finish = import_function(kernel, evaluate_unbounded_loop_finish,
            "unbounded_loop_finish()");
    }
}
} // namespace circa
