// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include <circa.h>

namespace circa {
namespace for_function {

    void format_heading(StyledSource* source, Term* term)
    {
        format_name_binding(source, term);
        append_phrase(source, "for ", term, phrase_type::KEYWORD);
        append_phrase(source, for_loop_get_iterator_name(term),
                term, phrase_type::UNDEFINED);
        append_phrase(source, " in ", term, phrase_type::KEYWORD);
        format_source_for_input(source, term, 0);
    }

    void formatSource(StyledSource* source, Term* term)
    {
        format_heading(source, term);
        format_branch_source(source, nested_contents(term), term);
        append_phrase(source, term->stringPropOptional("syntax:whitespaceBeforeEnd", ""),
            term, token::WHITESPACE);
    }

    CA_FUNCTION(evaluate_break)
    {
        while (top_frame(CONTEXT)->branch->owningTerm->function != FOR_FUNC)
            finish_frame(CONTEXT);
        finish_frame(CONTEXT);
    }
    CA_FUNCTION(evaluate_continue)
    {
        while (top_frame(CONTEXT)->branch->owningTerm->function != FOR_FUNC)
            finish_frame(CONTEXT);
        
        Term* index = for_loop_find_index(top_frame(CONTEXT)->branch);
        TaggedValue* indexVal = get_register(CONTEXT, index);
        set_int(indexVal, as_int(indexVal) + 1);
        top_frame(CONTEXT)->pc = 0;
    }
    CA_FUNCTION(evaluate_discard)
    {
        // FIXME
    }
    void break_formatSource(StyledSource* source, Term* term)
    {
        append_phrase(source, "break", term, phrase_type::KEYWORD);
    }
    void continue_formatSource(StyledSource* source, Term* term)
    {
        append_phrase(source, "continue", term, phrase_type::KEYWORD);
    }
    void discard_formatSource(StyledSource* source, Term* term)
    {
        append_phrase(source, "discard", term, phrase_type::KEYWORD);
    }

    void setup(Branch* kernel)
    {
        FOR_FUNC = import_function(kernel, evaluate_for_loop, "for(Indexable) -> List");
        as_function(FOR_FUNC)->formatSource = formatSource;
        as_function(FOR_FUNC)->createsStackFrame = true;
        as_function(FOR_FUNC)->vmInstruction = ControlFlowCall;

        BUILTIN_FUNCS.loop_iterator = import_function(kernel, NULL,
            "loop_iterator(any, any) -> int");
        BUILTIN_FUNCS.loop_index = import_function(kernel, NULL, "loop_index(any) -> int");
        BUILTIN_FUNCS.loop_output = import_function(kernel, evaluate_loop_output, "loop_output(int,any)");
        as_function(BUILTIN_FUNCS.loop_output)->vmInstruction = ControlFlowCall;

        DISCARD_FUNC = import_function(kernel, evaluate_discard, "discard(any)");
        as_function(DISCARD_FUNC)->formatSource = discard_formatSource;
        hide_from_docs(DISCARD_FUNC);

        BREAK_FUNC = import_function(kernel, evaluate_break, "break()");
        as_function(BREAK_FUNC)->formatSource = break_formatSource;
        as_function(BREAK_FUNC)->vmInstruction = ControlFlowCall;
        hide_from_docs(BREAK_FUNC);

        CONTINUE_FUNC = import_function(kernel, evaluate_continue, "continue()");
        as_function(CONTINUE_FUNC)->formatSource = continue_formatSource;
        as_function(CONTINUE_FUNC)->vmInstruction = ControlFlowCall;
        hide_from_docs(CONTINUE_FUNC);
    }
}
} // namespace circa
