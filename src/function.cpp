// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "building.h"
#include "code_iterators.h"
#include "control_flow.h"
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
#include "update_cascades.h"
#include "world.h"

namespace circa {

Function::Function()
  : declaringTerm(NULL),
    contents(NULL),
    feedbackFunc(NULL),
    throws(false),
    evaluate(NULL),
    specializeType(NULL),
    formatSource(NULL),
    checkInvariants(NULL),
    staticTypeQuery(NULL),
    onCreateCall(NULL),
    postInputChange(NULL),
    postCompile(NULL)
{
    gc_register_new_object((CircaObject*) this, TYPES.function, true);
}

Function::~Function()
{
    gc_on_object_deleted((CircaObject*) this);
}

namespace function_t {

    void initialize(Type* type, caValue* value)
    {
        Function* attrs = new Function();
        set_pointer(value, type, attrs);
    }

    void copy(Type* type, caValue* source, caValue* dest)
    {
        // Shallow copy.
        change_type(dest, type);
        dest->value_data = source->value_data;
    }

    void setup_type(Type* type)
    {
        set_string(&type->name, "Function");
        type->initialize = initialize;
        type->copy = copy;
        type->formatSource = function_format_source;
    }

} // namespace function_t

Block* function_contents(Term* func)
{
    return nested_contents(func);
}

Block* function_contents(Function* func)
{
    return nested_contents(func->declaringTerm);
}

Function* get_function_from_block(Block* block)
{
    return as_function(block->owningTerm);
}

Term* create_function(Block* block, const char* name)
{
    ca_assert(name != NULL);
    Term* term = create_value(block, TYPES.function, name);
    initialize_function(term);
    initialize_subroutine(term);
    return term;
}

void initialize_function(Term* func)
{
    term_value(func)->value_type = TYPES.function;
    as_function(func)->declaringTerm = func;
    as_function(func)->contents = nested_contents(func);
}

void finish_building_function(Block* contents)
{
    // Connect the primary output placeholder with the last expression.
    Term* primaryOutput = get_output_placeholder(contents, 0);
    ca_assert(primaryOutput->input(0) == NULL);
    Term* lastExpression = find_last_non_comment_expression(contents);
    set_input(primaryOutput, 0, lastExpression);

    // Make output type more specific.
    if (primaryOutput->type == TYPES.any && lastExpression != NULL)
        change_declared_type(primaryOutput, lastExpression->type);

    // Write a list of output_placeholder terms.

    // Look at every input declared as :output, these will be used to declare extra outputs.
    // TODO is a way to declare extra outputs that are not rebound inputs.
    for (int i = count_input_placeholders(contents) - 1; i >= 0; i--) {
        Term* input = get_input_placeholder(contents, i);

        if (input->boolProp("output", false)) {

            if (is_state_input(input)) {
                Term* term = append_state_output(contents);
                term->setIntProp("rebindsInput", i);
                continue;
            }

            Term* result = find_name(contents, input->name.c_str());
            
            Term* output = append_output_placeholder(contents, result);
            rename(output, input->nameSymbol);
            change_declared_type(output, input->type);
            output->setIntProp("rebindsInput", i);
        }
    }

    // After the output_placeholder terms are created, we might need to update any
    // recursive calls.

    for (BlockIterator it(contents); it.unfinished(); it.advance()) {
        Term* term = it.current();
        if (function_contents(term->function) != contents)
            continue;

        // Check if we need to insert a state input
        check_to_insert_implicit_inputs(term);

        // Update extra outputs
        update_extra_outputs(term);

        // Update cascade, might need to fix pack_state() calls.
        block_update_existing_pack_state_calls(term->owningBlock);
    }

    update_exit_points(contents);

    // Possibly apply a native patch
    module_possibly_patch_new_function(global_world(), contents);

    block_finish_changes(contents);
}

Type* derive_specialized_output_type(Term* function, Term* call)
{
    if (!is_function(function))
        return TYPES.any;

    Function* attrs = as_function(function);
    Type* outputType = function_get_output_type(attrs, 0);

    if (attrs->specializeType != NULL)
        outputType = attrs->specializeType(call);
    if (outputType == NULL)
        outputType = TYPES.any;

    if (function->boolProp("preferSpecialize", false)) {
        Term* specialized = statically_specialize_overload_for_call(call);
        if (specialized != NULL)
            return function_get_output_type(specialized, 0);
    }
    return outputType;
}

bool function_call_rebinds_input(Term* term, int index)
{
    return get_input_syntax_hint_optional(term, index, "rebindInput", "") == "t";
}

Type* function_get_input_type(Term* func, int index)
{
    return function_get_input_type(as_function(func), index);
}
Type* function_get_input_type(Function* func, int index)
{
    bool varArgs = has_variable_args(function_contents(func));
    if (varArgs)
        index = 0;

    Term* placeholder = function_get_input_placeholder(func, index);
    if (placeholder == NULL)
        return NULL;

    return placeholder->type;
}

Type* function_get_output_type(Term* function, int index)
{
    return function_get_output_type(as_function(function), index);
}

Type* function_get_output_type(Function* func, int index)
{
    if (func == NULL)
        return TYPES.any;

    // If there's no output_placeholder, then we are probably still building this
    // function.
    Term* placeholder = function_get_output_placeholder(func, index);
    if (placeholder == NULL)
        return TYPES.any;

    return placeholder->type;
}

Term* function_get_input_placeholder(Function* func, int index)
{
    Block* contents = function_get_contents(func);
    if (contents == NULL)
        return NULL;
    return get_input_placeholder(contents, index);
}

Term* function_get_output_placeholder(Function* func, int index)
{
    Block* contents = function_get_contents(func);
    if (contents == NULL)
        return NULL;
    return get_output_placeholder(contents, index);
}
Block* function_get_contents(Function* func)
{
    return func->contents;
}

std::string function_get_input_name(Function* func, int index)
{
    Term* placeholder = function_get_input_placeholder(func, index);
    if (placeholder == NULL)
        return "";
    return placeholder->name;
}

bool function_input_is_extra_output(Function* func, int index)
{
    return function_get_input_placeholder(func, index)->boolProp("output", false);
}

std::string function_get_documentation_string(Function* func)
{
    // A function can optionally have a documentation string. If present,
    // it will be the first thing defined in the function, and it'll be
    // anonymous and be a statement.
    Block* contents = function_get_contents(func);
    int expected_index = count_input_placeholders(contents);

    if (expected_index >= contents->length()) return "";
    Term* possibleDocString = contents->get(expected_index);
    if (possibleDocString->name != "") return "";
    if (!is_statement(possibleDocString)) return "";
    if (!is_value(possibleDocString)) return "";

    caValue* val = term_value(possibleDocString);
    if (!is_string(val)) return "";
    return as_string(val);
}

const char* get_output_name(Term* term, int outputIndex)
{
    if (outputIndex == 0)
        return term->name.c_str();

    Term* function = term->function;
    Function* attrs = NULL;

    if (function != NULL)
        attrs = as_function(function);

    if (attrs == NULL)
        return "";

    // If the call is rebinding an input name, then use that name.
    Term* outputPlaceholder = function_get_output_placeholder(attrs, outputIndex);
    int rebindsInput = outputPlaceholder->intProp("rebindsInput", -1);
    
    if (rebindsInput != -1 && rebindsInput < term->numInputs()) {
        Term* input = term->input(rebindsInput);
        if (input != NULL)
            return input->name.c_str();
    }

    return "";
}

void function_set_evaluate_func(Term* function, EvaluateFunc evaluate)
{
    as_function(function)->evaluate = evaluate;
}

void function_set_specialize_type_func(Term* func, SpecializeTypeFunc specializeFunc)
{
    as_function(func)->specializeType = specializeFunc;
}

void function_format_header_source(caValue* source, Block* function)
{
    Term* term = function->owningTerm;

    ca_assert(term != NULL);

    append_phrase(source, term->name, term, name_TermName);

    append_phrase(source, term->stringProp("syntax:postNameWs", ""),
            term, tok_Whitespace);
    append_phrase(source, term->stringProp("syntax:properties", ""),
            term, name_None);

    append_phrase(source, "(", term, tok_LParen);

    bool first = true;
    int numInputs = count_input_placeholders(function);
    for (int i=0; i < numInputs; i++) {

        Term* input = get_input_placeholder(function, i);

        std::string name = input->name;

        if (input->boolProp("hiddenInput", false))
            continue;

        if (input->boolProp("state", false))
            append_phrase(source, "state ", term, name_None);

        if (!first)
            append_phrase(source, ", ", term, name_None);
        first = false;

        // Type (may be omitted)
        if (input->boolProp("syntax:explicitType", true)) {
            append_phrase(source, as_cstring(&input->type->name),
                input->type->declaringTerm, name_TypeName);
            append_phrase(source, " ", term, tok_Whitespace);
        }

        // Name
        if (input->boolProp("syntax:rebindSymbol", false))
            append_phrase(source, "@", term, name_None);

        append_phrase(source, name, term, name_None);

        if (input->boolProp("output", false)
                && !input->boolProp("syntax:rebindSymbol", false)) {
            append_phrase(source, " ", term, tok_Whitespace);
            append_phrase(source, ":out", term, name_None);
        }

        if (input->boolProp("meta", false)) {
            append_phrase(source, " ", term, tok_Whitespace);
            append_phrase(source, ":meta", term, name_None);
        }

        if (input->boolProp("rebind", false)) {
            append_phrase(source, " ", term, tok_Whitespace);
            append_phrase(source, ":rebind", term, name_None);
        }

        if (input->boolProp("multiple", false)) {
            append_phrase(source, " ", term, tok_Whitespace);
            append_phrase(source, ":multiple", term, name_None);
        }
    }

    append_phrase(source, ")", term, tok_LParen);

    if (term->boolProp("syntax:explicitType", false)) {
        append_phrase(source, term->stringProp("syntax:whitespacePreColon", ""),
                term, tok_Whitespace);
        append_phrase(source, "->", term, name_None);
        append_phrase(source, term->stringProp("syntax:whitespacePostColon", ""),
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
            append_phrase(source, "(", term, name_None);

        for (int i=0;; i++) {

            Term* output = get_output_placeholder(function, i);
            if (output == NULL)
                break;
            if (is_hidden(output))
                continue;

            if (i > 0)
                append_phrase(source, ", ", term, name_None);

            append_phrase(source, as_cstring(&output->type->name),
                output->type->declaringTerm, name_TypeName);
        }

        if (multiOutputSyntax)
            append_phrase(source, ")", term, name_None);
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
    return as_function(term)->evaluate == evaluate_subroutine;
}

bool is_subroutine(Block* block)
{
    return block->owningTerm != NULL && is_subroutine(block->owningTerm);
}

void initialize_subroutine(Term* sub)
{
    // Install evaluate function
    as_function(sub)->evaluate = evaluate_subroutine;
}

} // namespace circa
