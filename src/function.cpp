// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "building.h"
#include "kernel.h"
#include "evaluation.h"
#include "function.h"
#include "introspection.h"
#include "source_repro.h"
#include "subroutine.h"
#include "term.h"
#include "term_list.h"
#include "token.h"
#include "type.h"
#include "types/list.h"
#include "update_cascades.h"

namespace circa {

Function::Function()
  : declaringTerm(NULL),
    contents(NULL),
    variableArgs(false),
    feedbackFunc(NULL),
    throws(false),
    createsStackFrame(false),
    evaluate(NULL),
    specializeType(NULL),
    formatSource(NULL),
    checkInvariants(NULL),
    staticTypeQuery(NULL),
    postInputChange(NULL),
    getOutputCount(NULL),
    getOutputName(NULL),
    getOutputType(NULL),
    assignRegisters(NULL),
    postCompile(NULL)
{
    register_new_object((CircaObject*) this, &FUNCTION_T, true);
}

Function::~Function()
{
    on_object_deleted((CircaObject*) this);
}

namespace function_t {

    void initialize(Type* type, TaggedValue* value)
    {
        Function* attrs = new Function();
        set_pointer(value, type, attrs);
    }

    void copy(Type* type, TaggedValue* source, TaggedValue* dest)
    {
        change_type(dest, type);
        *((Function*) get_pointer(dest)) = *((Function*) get_pointer(source));
    }

    void setup_type(Type* type)
    {
        type->name = "Function";
        type->initialize = initialize;
        type->copy = copy;
        type->formatSource = subroutine_f::format_source;
    }

} // namespace function_t

bool is_function(Term* term)
{
    if (term == NULL)
        return false;
    return term->type == &FUNCTION_T;
}

Branch* function_contents(Term* func)
{
    return nested_contents(func);
}

Branch* function_contents(Function* func)
{
    return nested_contents(func->declaringTerm);
}

Function* as_function(Term* func)
{
    if (func == NULL)
        return NULL;
    ca_assert(is_function(func));
    return (Function*) get_pointer(func);
}

std::string get_placeholder_name_for_index(int index)
{
    std::stringstream sstream;
    sstream << "#input_" << index;
    return sstream.str();
}

void initialize_function(Term* func)
{
    as_function(func)->declaringTerm = func;
    as_function(func)->contents = nested_contents(func);
}

void finish_building_function(Function* func, Type* declaredOutputType)
{
    // Write a list of output_placeholder terms.

    // Look at every input declared as :output, these will be used to declare extra outputs.
    // TODO is a way to declare extra outputs that are not rebound inputs.
    for (int i = function_num_inputs(func) - 1; i >= 0; i--) {
        Term* input = function_get_input_placeholder(func, i);

        if (input->boolPropOptional("state", false)) {
            Term* stateOutput = apply(function_contents(func),
                OUTPUT_PLACEHOLDER_FUNC, TermList(), input->name);
            stateOutput->setBoolProp("state", true);
            stateOutput->setIntProp("rebindsInput", i);
        }

        if (input->boolPropOptional("output", false)) {
            Term* output = apply(function_contents(func),
                OUTPUT_PLACEHOLDER_FUNC, TermList(), input->name);
            output->setIntProp("rebindsInput", i);
            change_declared_type(output, input->type);
        }
    }

    // Finally, write a final output_placeholder() term for the primary output.
    Term* output = apply(function_contents(func), OUTPUT_PLACEHOLDER_FUNC, TermList());
    change_declared_type(output, declaredOutputType);
}

bool is_callable(Term* term)
{
    return (term->type == &FUNCTION_T || term->type == &TYPE_T);
}

bool inputs_statically_fit_function(Term* func, TermList const& inputs)
{
    Function* funcAttrs = as_function(func);
    bool varArgs = funcAttrs->variableArgs;

    // Fail if wrong # of inputs
    if (!varArgs && (function_num_inputs(funcAttrs) != inputs.length()))
        return false;

    for (int i=0; i < inputs.length(); i++) {
        Type* type = function_get_input_type(func, i);
        Term* input = inputs[i];
        if (input == NULL)
            continue;

        bool alwaysSatisfiesType = term_output_always_satisfies_type(input, type);
        if (!alwaysSatisfiesType)
            return false;
    }

    return true;
}

bool inputs_fit_function_dynamic(Term* func, TermList const& inputs)
{
    Function* funcAttrs = as_function(func);
    bool varArgs = funcAttrs->variableArgs;

    // Fail if wrong # of inputs
    if (!varArgs && (function_num_inputs(funcAttrs) != inputs.length()))
        return false;

    for (int i=0; i < inputs.length(); i++) {
        Type* type = function_get_input_type(func, i);
        TaggedValue* value = inputs[i];
        if (value == NULL)
            continue;
        if (!cast_possible(value, type))
            return false;
    }
    return true;
}

bool values_fit_function_dynamic(Term* func, List* list)
{
    Function* funcAttrs = as_function(func);
    bool varArgs = funcAttrs->variableArgs;

    // Fail if wrong # of inputs
    if (!varArgs && (function_num_inputs(funcAttrs) != list->length()))
        return false;

    for (int i=0; i < list->length(); i++) {
        Type* type = function_get_input_type(func, i);
        TaggedValue* value = list->get(i);
        if (value == NULL)
            continue;
        if (!cast_possible(value, type))
            return false;
    }
    return true;
}

Term* create_overloaded_function(Branch* branch, std::string const& name,
        TermList const& overloads)
{
    return overloaded_function::create_overloaded_function(branch, name, overloads);
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
    if (funcAttrs->variableArgs)
        index = 0;

    Term* input = function_get_input_placeholder(funcAttrs, index);
    if (input == NULL)
        return false;
    return input->boolPropOptional("output", false);
}

bool function_implicitly_rebinds_input(Term* function, int index)
{
    Function* funcAttrs = as_function(function);
    Term* input = function_get_input_placeholder(funcAttrs, index);
    if (input == NULL)
        return false;
    return input->boolPropOptional("use-as-output", false);
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
    if (func->variableArgs)
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
        if (term == NULL || term->function != INPUT_PLACEHOLDER_FUNC)
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
        if (term == NULL || term->function != OUTPUT_PLACEHOLDER_FUNC)
            break;

        count++;
    }
    return count;
}

int function_num_explicit_inputs(Function* func)
{
    Branch* contents = function_get_contents(func);

    int count = 0;

    for (int i=0; i < contents->length(); i++) {
        if (contents->get(i) == NULL)
            break;
        if (contents->get(i)->function != INPUT_PLACEHOLDER_FUNC)
            break;

        if (!function_is_state_input(func, i))
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
    Branch* branch = function_get_contents(func);
    Term* term = apply(branch, INPUT_PLACEHOLDER_FUNC, TermList());
    branch->move(term, 0);
    term->setBoolProp("state", true);
    return term;
}

Term* function_find_state_input(Function* func)
{
    for (int i=0;; i++) {
        Term* placeholder = function_get_input_placeholder(func,i);
        if (placeholder == NULL)
            return NULL;
        if (function_is_state_input(placeholder))
            return placeholder;
    }
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

Term* function_get_input_placeholder(Function* func, int index)
{
    Branch* contents = function_get_contents(func);
    if (contents == NULL)
        return NULL;
    if (index >= contents->length())
        return NULL;
    Term* term = contents->get(index);
    if (term->function != INPUT_PLACEHOLDER_FUNC)
        return NULL;
    return term;
}

Term* function_get_output_placeholder(Function* func, int index)
{
    Branch* contents = function_get_contents(func);
    if (contents == NULL)
        return NULL;
    if (index >= contents->length())
        return NULL;
    Term* term = contents->getFromEnd(index);
    if (term->function != OUTPUT_PLACEHOLDER_FUNC)
        return NULL;
    return term;
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
    if (!is_string(possibleDocString)) return "";
    if (!is_statement(possibleDocString)) return "";
    if (!is_value(possibleDocString)) return "";
    return as_string(possibleDocString);
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
    
    if (rebindsInput != -1)
        return term->input(rebindsInput)->name.c_str();

#if 0
    // First we need to figure out what the corresponding input index is for this
    // rebound output.
    int checkOutputIndex = 1;
    int reboundInputIndex = -1;
    for (int inputIndex=0; inputIndex < function_num_inputs(attrs); inputIndex++) {
        if (function_can_rebind_input(function, inputIndex)) {
            if (function_call_rebinds_input(term, inputIndex)
                    && (checkOutputIndex == outputIndex)) {
                reboundInputIndex = inputIndex;
                break;
            }
            checkOutputIndex++;
        }
    }

    // If a rebound input was found, use that name.
    if (reboundInputIndex != -1)
        return get_output_name(term->input(reboundInputIndex), 0);
#endif

    return "";
}

bool is_native_function(Function* func)
{
    return func->evaluate != evaluate_subroutine;
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
            term, token::WHITESPACE);
    append_phrase(source, term->stringPropOptional("syntax:properties", ""),
            term, phrase_type::UNDEFINED);

    append_phrase(source, "(", term, token::LPAREN);

    bool first = true;
    int numInputs = function_num_inputs(func);
    for (int i=0; i < numInputs; i++) {
        std::string name = function_get_input_name(func, i);

        if (name == "#state")
            continue;

        Term* input = function_get_input_placeholder(func, i);

        if (input->boolPropOptional("state", false))
            append_phrase(source, "state ", term, phrase_type::UNDEFINED);

        if (!first)
            append_phrase(source, ", ", term, phrase_type::UNDEFINED);
        first = false;

        if (!first)
            append_phrase(source, function_get_input_type(term, i)->name,
                    term, phrase_type::TYPE_NAME);

        if (name != "" && name[0] != '#') {
            append_phrase(source, " ", term, token::WHITESPACE);
            append_phrase(source, name, term, phrase_type::UNDEFINED);
        }

        if (function_can_rebind_input(term, i)) {
            append_phrase(source, " ", term, token::WHITESPACE);
            append_phrase(source, ":out", term, phrase_type::UNDEFINED);
        }

        if (input->boolPropOptional("meta", false)) {
            append_phrase(source, " ", term, token::WHITESPACE);
            append_phrase(source, ":meta", term, phrase_type::UNDEFINED);
        }
    }

    if (func->variableArgs)
        append_phrase(source, "...", term, phrase_type::UNDEFINED);

    append_phrase(source, ")", term, token::LPAREN);

    if (function_get_output_type(term, 0) != &VOID_T) {
        append_phrase(source, term->stringPropOptional("syntax:whitespacePreColon", ""),
                term, token::WHITESPACE);
        append_phrase(source, "->", term, phrase_type::UNDEFINED);
        append_phrase(source, term->stringPropOptional("syntax:whitespacePostColon", ""),
                term, token::WHITESPACE);
        append_phrase(source, function_get_output_type(term, 0)->name,
                term, phrase_type::TYPE_NAME);
    }
}

} // namespace circa
