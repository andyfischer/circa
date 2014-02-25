// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "building.h"
#include "code_iterators.h"
#include "control_flow.h"
#include "closures.h"
#include "function.h"
#include "generic.h"
#include "kernel.h"
#include "inspection.h"
#include "interpreter.h"
#include "list.h"
#include "native_patch.h"
#include "source_repro.h"
#include "stateful_code.h"
#include "string_type.h"
#include "names.h"
#include "term.h"
#include "term_list.h"
#include "token.h"
#include "type.h"
#include "type_inference.h"
#include "update_cascades.h"
#include "world.h"

namespace circa {

Term* create_function(Block* block, const char* name)
{
    ca_assert(name != NULL);
    Term* term = apply(block, FUNCS.function_decl, TermList(), name);
    return term;
}

Type* find_implicit_output_type(Block* block)
{
    Type* type = declared_type(get_output_placeholder(block, 0)->input(0));

    // The implicit type must also consider any return statements.
    for (MinorBlockIterator it(block); it; ++it) {
        Term* term = *it;
        if (term->function == FUNCS.return_func)
            type = find_common_type(type, declared_type(term->input(0)));
    }
    return type;
}

void finish_building_function(Block* contents)
{
    // Connect the primary output placeholder with the last expression.
    Term* primaryOutput = get_output_placeholder(contents, 0);
    ca_assert(primaryOutput->input(0) == NULL);
    Term* lastExpression = find_expression_for_implicit_output(contents);
    if (lastExpression != NULL) {
        set_input(primaryOutput, 0, lastExpression);
        if (!primaryOutput->boolProp(sym_ExplicitType, false))
            set_declared_type(primaryOutput, find_implicit_output_type(contents));
    }

    // Write a list of output_placeholder terms.

    // Look at every input declared as :output, these will be used to declare extra outputs.
    // TODO is a way to declare extra outputs that are not rebound inputs.
    for (int i = count_input_placeholders(contents) - 1; i >= 0; i--) {
        Term* input = get_input_placeholder(contents, i);

        if (input->boolProp(sym_Output, false)) {

            Term* result = find_name(contents, input->name.c_str());
            
            Term* output = append_output_placeholder(contents, result);
            rename(output, &input->nameValue);
            set_declared_type(output, input->type);
            output->setIntProp(sym_RebindsInput, i);
        }
    }

    // After the output_placeholder terms are created, we might need to update any
    // recursive calls.

    for (BlockIterator it(contents); it.unfinished(); it.advance()) {
        Term* term = it.current();
        if (function_contents(term->function) != contents)
            continue;

        // Update extra outputs
        update_extra_outputs(term);
    }

    update_for_control_flow(contents);
    insert_nonlocal_terms(contents);

    // Possibly apply a native patch
    native_patch_apply_to_new_function(global_world(), contents);

    block_finish_changes(contents);
}

Type* derive_specialized_output_type(Term* function, Term* call)
{
    if (!is_function(function))
        return TYPES.any;

    Block* contents = function_contents(function);
    Type* outputType = get_output_type(contents, 0);

    if (contents->overrides.specializeType != NULL)
        outputType = contents->overrides.specializeType(call);
    if (outputType == NULL)
        outputType = TYPES.any;

    if (function->boolProp(sym_PreferSpecialize, false)) {
        Term* specialized = statically_specialize_overload_for_call(call);
        if (specialized != NULL)
            return get_output_type(function_contents(specialized), 0);
    }
    return outputType;
}

bool function_call_rebinds_input(Term* term, int index)
{
    return get_input_syntax_hint_optional(term, index, sym_RebindsInput, "") == "t";
}

void function_format_header_source(caValue* source, Block* function)
{
    Term* term = function->owningTerm;

    ca_assert(term != NULL);

    append_phrase(source, term->name, term, sym_TermName);

    append_phrase(source, term->stringProp(sym_Syntax_PostNameWs, ""),
            term, tok_Whitespace);
    append_phrase(source, term->stringProp(sym_Syntax_Properties, ""),
            term, sym_None);

    append_phrase(source, "(", term, tok_LParen);

    bool first = true;
    int numInputs = count_input_placeholders(function);
    for (int i=0; i < numInputs; i++) {

        Term* input = get_input_placeholder(function, i);

        std::string name = input->name;

        if (input->boolProp(sym_HiddenInput, false))
            continue;

        if (input->boolProp(sym_State, false))
            append_phrase(source, "state ", term, sym_None);

        if (!first)
            append_phrase(source, ", ", term, sym_None);
        first = false;

        // Type (may be omitted)
        if (input->boolProp(sym_Syntax_ExplicitType, true)) {
            append_phrase(source, as_cstring(&input->type->name),
                input->type->declaringTerm, sym_TypeName);
            append_phrase(source, " ", term, tok_Whitespace);
        }

        // Name
        if (input->boolProp(sym_Syntax_RebindSymbol, false))
            append_phrase(source, "@", term, sym_None);

        append_phrase(source, name, term, sym_None);

        if (input->boolProp(sym_Output, false)
                && !input->boolProp(sym_Syntax_RebindSymbol, false)) {
            append_phrase(source, " ", term, tok_Whitespace);
            append_phrase(source, ":out", term, sym_None);
        }

        if (input->boolProp(sym_Meta, false)) {
            append_phrase(source, " ", term, tok_Whitespace);
            append_phrase(source, ":meta", term, sym_None);
        }

        if (input->boolProp(sym_Rebind, false)) {
            append_phrase(source, " ", term, tok_Whitespace);
            append_phrase(source, ":rebind", term, sym_None);
        }

        if (input->boolProp(sym_Multiple, false)) {
            append_phrase(source, " ", term, tok_Whitespace);
            append_phrase(source, ":multiple", term, sym_None);
        }
    }

    append_phrase(source, ")", term, tok_LParen);

    if (term->boolProp(sym_Syntax_ExplicitType, false)) {
        append_phrase(source, term->stringProp(sym_Syntax_WhitespacePreColon, ""),
                term, tok_Whitespace);
        append_phrase(source, "->", term, sym_None);
        append_phrase(source, term->stringProp(sym_Syntax_WhitespacePostColon, ""),
                term, tok_Whitespace);

        int unhiddenOutputCount = 0;

        for (int i=0;; i++) {
            Term* output = get_output_placeholder(function, i);
            if (output == NULL)
                break;
            if (is_hidden(output))
                continue;
            unhiddenOutputCount++;
        }

        bool multiOutputSyntax = unhiddenOutputCount > 1;

        if (multiOutputSyntax)
            append_phrase(source, "(", term, sym_None);

        for (int i=0;; i++) {

            Term* output = get_output_placeholder(function, i);
            if (output == NULL)
                break;
            if (is_hidden(output))
                continue;

            if (i > 0)
                append_phrase(source, ", ", term, sym_None);

            append_phrase(source, as_cstring(&output->type->name),
                output->type->declaringTerm, sym_TypeName);
        }

        if (multiOutputSyntax)
            append_phrase(source, ")", term, sym_None);
    }
}

void function_format_source(caValue* source, Term* term)
{
    append_phrase(source, "def ", term, tok_Def);

    Block* contents = function_contents(term);

    function_format_header_source(source, contents);
    format_block_source(source, contents, term);
}

void evaluate_subroutine(caStack*)
{
    // This once did something, but now the default function calling behavior
    // is the same as evaluating a subroutine.
}

bool is_subroutine(Term* term)
{
    if (!is_function(term))
        return false;
    return function_contents(term)->overrides.evaluate == NULL;
}

bool is_subroutine(Block* block)
{
    return block->owningTerm != NULL && is_subroutine(block->owningTerm);
}

} // namespace circa
