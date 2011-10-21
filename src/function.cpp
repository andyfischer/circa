// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "building.h"
#include "builtins.h"
#include "evaluation.h"
#include "function.h"
#include "heap_debugging.h"
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

FunctionAttrs::FunctionAttrs()
  : declaringTerm(NULL),
    implicitStateType(NULL),
    variableArgs(false),
    feedbackFunc(NULL),
    throws(false),
    outputCount(1),
    simpleEvaluate(NULL),
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
    debug_register_valid_object(this, FUNCTION_ATTRS_OBJECT);
}

FunctionAttrs::~FunctionAttrs()
{
    debug_unregister_valid_object(this, FUNCTION_ATTRS_OBJECT);
}

namespace function_attrs_t {

    void initialize(Type* type, TaggedValue* value)
    {
        FunctionAttrs* attrs = new FunctionAttrs();
        set_pointer(value, type, attrs);
    }

    void release(Type*, TaggedValue* value)
    {
        delete (FunctionAttrs*) get_pointer(value);
    }

    void copy(Type* type, TaggedValue* source, TaggedValue* dest)
    {
        change_type(dest, type);
        *((FunctionAttrs*) get_pointer(dest)) = *((FunctionAttrs*) get_pointer(source));
    }

} // namespace function_attrs_t

namespace function_t {

    void setup_type(Type* type)
    {
        type->name = "Function";
        type->formatSource = subroutine_f::format_source;
        type->storageType = STORAGE_TYPE_REF;
    }

} // namespace function_t

bool is_function(Term* term)
{
    if (term == NULL)
        return false;
    return term->type == &FUNCTION_T;
}

bool is_function_attrs(Term* term)
{
    return term->type == &FUNCTION_ATTRS_T;
}

FunctionAttrs& as_function_attrs(Term* term)
{
    ca_assert(is_function_attrs(term));
    ca_assert(get_pointer(term) != NULL);
    return *((FunctionAttrs*) get_pointer(term));
}

Branch* function_contents(Term* func)
{
    return nested_contents(func);
}

Branch* function_contents(FunctionAttrs* func)
{
    return nested_contents(func->declaringTerm);
}

FunctionAttrs* get_function_attrs(Term* func)
{
    if (func == NULL)
        return NULL;
    if (nested_contents(func)->length() == 0)
        return NULL;
    if (nested_contents(func)->get(0)->type != &FUNCTION_ATTRS_T)
        return NULL;
    return &as_function_attrs(nested_contents(func)->get(0));
}

std::string get_placeholder_name_for_index(int index)
{
    std::stringstream sstream;
    sstream << "#input_" << index;
    return sstream.str();
}

void initialize_function(Term* func)
{
    /* A function has a branch with the following structures:
      {
        [0] FunctionAttrs #attributes
        [1..num_inputs] input terms
            .. each might have bool property 'modified' or 'meta'
        [...] function body
        [n-1] output term, type is significant
      }
    */

    Term* attributesTerm = create_value(nested_contents(func), &FUNCTION_ATTRS_T, "#attributes");
    hide_from_source(attributesTerm);

    // Setup the term's global value to point back to the term, so that the function
    // can be passed as a value. (Deprecated in favor of declaringTerm)
    set_function_pointer(func, func);

    get_function_attrs(func)->declaringTerm = func;
}

void finish_parsing_function_header(Term* func)
{
    // Called by parser when we finish reading this function's list of inputs.
    //
    // Here we'll look at every input declared as +out, and we'll update the function's
    // outputTypes and outputCount.

    FunctionAttrs* attrs = get_function_attrs(func);
    attrs->outputCount = 1;
    attrs->outputTypes.resize(1);

    for (int i=0; i < function_num_inputs(attrs); i++) {
        Term* input = function_get_input_placeholder(attrs, i);
        if (input->boolPropOptional("output", false)) {
            attrs->outputCount++;
            set_type(attrs->outputTypes.append(), get_output_type(input));
        }
    }
}

bool is_callable(Term* term)
{
    return (term->type == &FUNCTION_T || term->type == &TYPE_T);
}

bool inputs_statically_fit_function(Term* func, TermList const& inputs)
{
    FunctionAttrs* funcAttrs = get_function_attrs(func);
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
    FunctionAttrs* funcAttrs = get_function_attrs(func);
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
    FunctionAttrs* funcAttrs = get_function_attrs(func);
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
    FunctionAttrs* attrs = get_function_attrs(function);

    if (!FINISHED_BOOTSTRAP)
        return &ANY_T;
    if (!is_function(function))
        return &ANY_T;
    Type* outputType = function_get_output_type(attrs, 0);

    if (attrs->specializeType != NULL)
        outputType = attrs->specializeType(call);
    if (outputType == NULL)
        outputType = &ANY_T;
    return outputType;
}

bool function_can_rebind_input(Term* func, int index)
{
    FunctionAttrs* funcAttrs = get_function_attrs(func);
    if (funcAttrs->variableArgs)
        index = 0;

    Term* input = function_get_input_placeholder(funcAttrs, index);
    if (input == NULL)
        return false;
    return input->boolPropOptional("output", false);
}

bool function_implicitly_rebinds_input(Term* function, int index)
{
    FunctionAttrs* funcAttrs = get_function_attrs(function);
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
    return function_get_input_type(get_function_attrs(func), index);
}
Type* function_get_input_type(FunctionAttrs* func, int index)
{
    if (func->variableArgs)
        index = 0;
    return function_get_input_placeholder(func, index)->type;
}

Type* function_get_output_type(Term* function, int index)
{
    return function_get_output_type(get_function_attrs(function), index);
}

Type* function_get_output_type(FunctionAttrs* func, int index)
{
    if (func == NULL)
        return &ANY_T;

    ca_assert(index < func->outputTypes.length());

    return as_type(func->outputTypes[index]);
}

int function_num_inputs(FunctionAttrs* func)
{
    Branch* contents = nested_contents(func->declaringTerm);
    int i = 1;

    while (i < contents->length()
            && contents->get(i) != NULL
            && contents->get(i)->function == INPUT_PLACEHOLDER_FUNC)
        i++;
    return i - 1;
}

bool function_is_state_input(FunctionAttrs* func, int index)
{
    Term* placeholder = function_get_input_placeholder(func,index);
    if (placeholder == NULL)
        return false;
    return placeholder->boolPropOptional("state", false);
}    
bool function_get_input_meta(FunctionAttrs* func, int index)
{
    Term* placeholder = function_get_input_placeholder(func, index);
    if (placeholder == NULL)
        return false;
    return placeholder->boolPropOptional("meta", false);
}
bool function_get_input_optional(FunctionAttrs* func, int index)
{
    Term* placeholder = function_get_input_placeholder(func, index);
    if (placeholder == NULL)
        return false;
    return placeholder->boolPropOptional("optional", false);
}

Term* function_get_input_placeholder(FunctionAttrs* func, int index)
{
    Branch* contents = function_get_contents(func);
    index += 1;
    if (index >= contents->length())
        return NULL;
    return contents->get(index);
}
Branch* function_get_contents(FunctionAttrs* func)
{
    return nested_contents(func->declaringTerm);
}

std::string function_get_input_name(FunctionAttrs* func, int index)
{
    Term* placeholder = function_get_input_placeholder(func, index);
    if (placeholder == NULL)
        return "";
    return placeholder->name;
}

std::string function_get_documentation_string(FunctionAttrs* func)
{
    // A function can optionally have a documentation string. If present,
    // it will be the first thing defined in the function, and it'll be
    // anonymous and be a statement.
    int expected_index = function_num_inputs(func) + 1;
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
    FunctionAttrs* attrs = NULL;

    if (function != NULL)
        attrs = get_function_attrs(function);

    if (attrs == NULL)
        return "";

    FunctionAttrs::GetOutputName getOutputName = attrs->getOutputName;

    if (getOutputName != NULL)
        return getOutputName(term, outputIndex);

    // Default behavior, if the call is rebinding an input name, then use that name.

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
        return get_output_name_for_input(term, reboundInputIndex);

    return "";
}

const char* get_output_name_for_input(Term* term, int inputIndex)
{
    return get_output_name(term->input(inputIndex),
            term->inputInfo(inputIndex)->outputIndex);
}

bool is_native_function(FunctionAttrs* func)
{
    return func->evaluate != evaluate_subroutine;
}

void function_set_evaluate_func(Term* function, EvaluateFunc evaluate)
{
    get_function_attrs(function)->evaluate = evaluate;
}

void function_set_specialize_type_func(Term* func, SpecializeTypeFunc specializeFunc)
{
    get_function_attrs(func)->specializeType = specializeFunc;
}

void function_format_header_source(StyledSource* source, FunctionAttrs* func)
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
