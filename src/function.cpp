// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "building.h"
#include "control_flow.h"
#include "evaluation.h"
#include "function.h"
#include "kernel.h"
#include "introspection.h"
#include "list.h"
#include "source_repro.h"
#include "stateful_code.h"
#include "names.h"
#include "term.h"
#include "term_list.h"
#include "token.h"
#include "type.h"
#include "update_cascades.h"

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
    getOutputCount(NULL),
    getOutputName(NULL),
    getOutputType(NULL),
    assignRegisters(NULL),
    postCompile(NULL)
{
    gc_register_new_object((CircaObject*) this, &FUNCTION_T, true);
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
        type->name = name_from_string("Function");
        type->initialize = initialize;
        type->copy = copy;
        type->formatSource = function_format_source;
    }

} // namespace function_t

Branch* function_contents(Term* func)
{
    return nested_contents(func);
}

Branch* function_contents(Function* func)
{
    return nested_contents(func->declaringTerm);
}

std::string get_placeholder_name_for_index(int index)
{
    std::stringstream sstream;
    sstream << "#input_" << index;
    return sstream.str();
}

Term* create_function(Branch* branch, const char* name)
{
    Term* term = create_value(branch, &FUNCTION_T, name);
    initialize_function(term);
    initialize_subroutine(term);
    return term;
}

void initialize_function(Term* func)
{
    term_value(func)->value_type = &FUNCTION_T;
    as_function(func)->declaringTerm = func;
    as_function(func)->contents = nested_contents(func);
}

void finish_building_function(Function* func, Type* declaredOutputType)
{
    Branch* contents = function_contents(func);

    // Write a list of output_placeholder terms.

    // Look at every input declared as :output, these will be used to declare extra outputs.
    // TODO is a way to declare extra outputs that are not rebound inputs.
    for (int i = function_num_inputs(func) - 1; i >= 0; i--) {
        Term* input = function_get_input_placeholder(func, i);

        if (input->boolPropOptional("output", false)) {

            if (is_state_input(input)) {
                Term* term = insert_state_output(contents);
                term->setIntProp("rebindsInput", i);
                continue;
            }

            Term* result = find_name(contents, input->name.c_str());
            Term* output = apply(contents,
                FUNCS.output, TermList(result), input->name);
            change_declared_type(output, input->type);
            output->setIntProp("rebindsInput", i);
        }
    }

    // Finally, write a final output_placeholder() term for the primary output.
    Term* output = apply(contents, FUNCS.output, TermList(NULL));
    change_declared_type(output, declaredOutputType);

    update_exit_points(contents);
}

bool inputs_fit_function_dynamic(Term* func, TermList const& inputs)
{
    Function* funcAttrs = as_function(func);
    bool varArgs = function_has_variable_args(func);

    // Fail if wrong # of inputs
    if (!varArgs && (function_num_inputs(funcAttrs) != inputs.length()))
        return false;

    for (int i=0; i < inputs.length(); i++) {
        Type* type = function_get_input_type(func, i);
        caValue* value = term_value(inputs[i]);
        if (value == NULL)
            continue;
        if (!cast_possible(value, type))
            return false;
    }
    return true;
}

Type* derive_specialized_output_type(Term* function, Term* call)
{
    if (!FINISHED_BOOTSTRAP)
        return &ANY_T;
    if (!is_function(function))
        return &ANY_T;

    Function* attrs = as_function(function);
    Type* outputType = function_get_output_type(attrs, 0);

    if (attrs->specializeType != NULL)
        outputType = attrs->specializeType(call);
    if (outputType == NULL)
        outputType = &ANY_T;
    return outputType;
}

bool function_can_rebind_input(Term* func, int index)
{
    Function* funcAttrs = as_function(func);

    Term* input = function_get_input_placeholder(funcAttrs, index);
    if (input == NULL)
        return false;
    return input->boolPropOptional("output", false);
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
    bool varArgs = function_has_variable_args(func);
    if (varArgs)
        index = 0;
    return function_get_input_placeholder(func, index)->type;
}

Type* function_get_output_type(Term* function, int index)
{
    return function_get_output_type(as_function(function), index);
}

Type* function_get_output_type(Function* func, int index)
{
    if (func == NULL)
        return &ANY_T;

    // If there's no output_placeholder, then we are probably still building this
    // function.
    Term* placeholder = function_get_output_placeholder(func, index);
    if (placeholder == NULL)
        return &ANY_T;

    return placeholder->type;
}

int function_num_inputs(Function* func)
{
    Branch* contents = function_get_contents(func);

    int count = 0;
    for (int i=0; i < contents->length(); i++) {
        Term* term = contents->get(i);
        if (term == NULL || term->function != FUNCS.input)
            break;

        count++;
    }
    return count;
}

int function_num_outputs(Function* func)
{
    Branch* contents = function_get_contents(func);

    int count = 0;
    for (int i=contents->length() - 1; i >= 0; i--) {
        Term* term = contents->get(i);
        if (term == NULL || term->function != FUNCS.output)
            break;

        count++;
    }
    return count;
}

bool function_is_state_input(Term* placeholder)
{
    return placeholder->boolPropOptional("state", false);
}

bool function_is_state_input(Function* func, int index)
{
    Term* placeholder = function_get_input_placeholder(func,index);
    if (placeholder == NULL)
        return false;
    return function_is_state_input(placeholder);
}

bool function_has_state_input(Function* func)
{
    // Walk through inputs, try to find a stateful input.
    int index = 0;
    while (true) {
        Term* placeholder = function_get_input_placeholder(func, index++);
        if (placeholder == NULL)
            return false;
        if (function_is_state_input(placeholder))
            return true;
    }
}    

Term* function_insert_state_input(Function* func)
{
    return append_state_input(function_contents(func));
}

bool function_is_multiple_input(Term* placeholder)
{
    return placeholder->boolPropOptional("multiple", false);
}

bool function_is_multiple_input(Function* func, int index)
{
    Term* placeholder = function_get_input_placeholder(func, index);
    if (placeholder == NULL)
        return false;
    return function_is_multiple_input(placeholder);
}

bool function_get_input_meta(Function* func, int index)
{
    Term* placeholder = function_get_input_placeholder(func, index);
    if (placeholder == NULL)
        return false;
    return placeholder->boolPropOptional("meta", false);
}
bool function_get_input_optional(Function* func, int index)
{
    Term* placeholder = function_get_input_placeholder(func, index);
    if (placeholder == NULL)
        return false;
    return placeholder->boolPropOptional("optional", false);
}
bool function_has_variable_args(Function* func)
{
    return has_variable_args(function_get_contents(func));
}
bool function_has_variable_args(Term* func)
{
    return function_has_variable_args(as_function(func));
}

Term* function_get_input_placeholder(Function* func, int index)
{
    Branch* contents = function_get_contents(func);
    if (contents == NULL)
        return NULL;
    return get_input_placeholder(contents, index);
}

Term* function_get_output_placeholder(Function* func, int index)
{
    Branch* contents = function_get_contents(func);
    if (contents == NULL)
        return NULL;
    return get_output_placeholder(contents, index);
}
Branch* function_get_contents(Function* func)
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
    return function_get_input_placeholder(func, index)->boolPropOptional("output", false);
}

std::string function_get_documentation_string(Function* func)
{
    // A function can optionally have a documentation string. If present,
    // it will be the first thing defined in the function, and it'll be
    // anonymous and be a statement.
    int expected_index = function_num_inputs(func);
    Branch* contents = function_get_contents(func);

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

    Function::GetOutputName getOutputName = attrs->getOutputName;

    if (getOutputName != NULL)
        return getOutputName(term, outputIndex);

    // Default behavior, if the call is rebinding an input name, then use that name.
    Term* outputPlaceholder = function_get_output_placeholder(attrs, outputIndex);
    int rebindsInput = outputPlaceholder->intPropOptional("rebindsInput", -1);
    
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

void function_format_header_source(StyledSource* source, Function* func)
{
    Term* term = func->declaringTerm;
    ca_assert(term != NULL);

    append_phrase(source, term->name, term, phrase_type::TERM_NAME);

    append_phrase(source, term->stringPropOptional("syntax:postNameWs", ""),
            term, TK_WHITESPACE);
    append_phrase(source, term->stringPropOptional("syntax:properties", ""),
            term, phrase_type::UNDEFINED);

    append_phrase(source, "(", term, TK_LPAREN);

    bool first = true;
    int numInputs = function_num_inputs(func);
    for (int i=0; i < numInputs; i++) {
        std::string name = function_get_input_name(func, i);

        Term* input = function_get_input_placeholder(func, i);

        if (input->boolPropOptional("hiddenInput", false))
            continue;

        if (input->boolPropOptional("state", false))
            append_phrase(source, "state ", term, phrase_type::UNDEFINED);

        if (!first)
            append_phrase(source, ", ", term, phrase_type::UNDEFINED);
        first = false;

        bool showType = true;
        if (i == 0 && term->boolPropOptional("syntax:methodDecl", false))
            showType = false;

        if (showType)
            append_phrase(source,
                name_to_string(function_get_input_type(term, i)->name),
                term, phrase_type::TYPE_NAME);

        if (name != "" && name[0] != '#') {
            if (showType)
                append_phrase(source, " ", term, TK_WHITESPACE);
            append_phrase(source, name, term, phrase_type::UNDEFINED);
        }

        if (function_can_rebind_input(term, i)) {
            append_phrase(source, " ", term, TK_WHITESPACE);
            append_phrase(source, ":out", term, phrase_type::UNDEFINED);
        }

        if (input->boolPropOptional("meta", false)) {
            append_phrase(source, " ", term, TK_WHITESPACE);
            append_phrase(source, ":meta", term, phrase_type::UNDEFINED);
        }

        if (input->boolPropOptional("rebind", false)) {
            append_phrase(source, " ", term, TK_WHITESPACE);
            append_phrase(source, ":rebind", term, phrase_type::UNDEFINED);
        }
    }

    bool varArgs = function_has_variable_args(func);
    if (varArgs)
        append_phrase(source, "...", term, phrase_type::UNDEFINED);

    append_phrase(source, ")", term, TK_LPAREN);

    if (function_get_output_type(term, 0) != &VOID_T) {
        append_phrase(source, term->stringPropOptional("syntax:whitespacePreColon", ""),
                term, TK_WHITESPACE);
        append_phrase(source, "->", term, phrase_type::UNDEFINED);
        append_phrase(source, term->stringPropOptional("syntax:whitespacePostColon", ""),
                term, TK_WHITESPACE);
        append_phrase(source,
            name_to_string(function_get_output_type(term, 0)->name),
                term, phrase_type::TYPE_NAME);
    }
}

void function_format_source(StyledSource* source, Term* term)
{
    append_phrase(source, "def ", term, TK_DEF);

    Function* func = as_function(term);

    function_format_header_source(source, func);

    format_branch_source(source, nested_contents(term), term);
}

void function_set_empty_evaluation(Function* function)
{
    function_contents(function)->emptyEvaluation = true;
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

void initialize_subroutine(Term* sub)
{
    // Install evaluate function
    as_function(sub)->evaluate = evaluate_subroutine;
}

} // namespace circa
