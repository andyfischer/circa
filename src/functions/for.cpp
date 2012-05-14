// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "circa/internal/for_hosted_funcs.h"

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
            term, TK_WHITESPACE);
    }

    CA_FUNCTION(evaluate_break)
    {
        while (top_frame(CONTEXT)->branch->owningTerm->function != FUNCS.for_func)
            finish_frame(CONTEXT);

        finish_frame(CONTEXT);
    }
    CA_FUNCTION(evaluate_continue)
    {
        // Save args before we pop the frame
        Value args;
        circa_copy(circa_input(STACK, 0), &args);

        // Pop frames
        while (top_frame(CONTEXT)->branch->owningTerm->function != FUNCS.for_func)
            finish_frame(CONTEXT);

        // Copy outputs to placeholders
        Branch* branch = top_frame(CONTEXT)->branch;
        for (int i=0;; i++) {
            // Fetch placeholder + 1 (skip the primary output)
            Term* output = get_output_placeholder(branch, i + 1);
            if (output == NULL)
                break;

            caValue* out = get_register(CONTEXT, output);

            if (i >= circa_count(&args))
                set_null(out);
            else
                move(circa_index(&args, i), out);
        }

        for_loop_finish_iteration(CONTEXT);
    }
    void evaluate_discard(caStack* stack)
    {
        while (top_frame(stack)->branch->owningTerm->function != FUNCS.for_func) {
            finish_frame(stack);

            //if (error_occurred(stack))
                //return;
        }

        for_loop_finish_iteration(stack);
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
        FUNCS.for_func = import_function(kernel, NULL, "for(Indexable) -> List");
        as_function(FUNCS.for_func)->formatSource = formatSource;

        FUNCS.loop_output = import_function(kernel, NULL, "loop_output(any) -> List");
        FUNCS.loop_iterator = import_function(kernel, NULL,
            "loop_iterator(any, any) -> int");
        FUNCS.loop_index = import_function(kernel, NULL, "loop_index(any) -> int");
        function_set_empty_evaluation(as_function(FUNCS.loop_index));

        FUNCS.discard = import_function(kernel, evaluate_discard, "discard()");
        as_function(FUNCS.discard)->formatSource = discard_formatSource;
        hide_from_docs(FUNCS.discard);

        FUNCS.break_func = import_function(kernel, evaluate_break,
            "break(any :multiple :optional)");
        as_function(FUNCS.break_func)->formatSource = break_formatSource;
        hide_from_docs(FUNCS.break_func);

        FUNCS.continue_func = import_function(kernel, evaluate_continue,
            "continue(any :multiple :optional)");
        as_function(FUNCS.continue_func)->formatSource = continue_formatSource;
        hide_from_docs(FUNCS.continue_func);

        FUNCS.unbounded_loop = import_function(kernel, evaluate_unbounded_loop,
            "loop(bool condition)");
        FUNCS.unbounded_loop_finish = import_function(kernel, evaluate_unbounded_loop_finish,
            "unbounded_loop_finish()");
    }
}
} // namespace circa
