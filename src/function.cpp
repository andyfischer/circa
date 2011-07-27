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
  : implicitStateType(NULL),
    variableArgs(false),
    feedbackFunc(NULL),
    throws(false),
    outputCount(1),
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
    std::string to_string(Term* term)
    {
        return "<Function " + function_t::get_name(term) + ">";
    }

    std::string get_header_source(Term* term)
    {
        std::stringstream out;

        out << term->name;

        out << term->stringPropOptional("syntax:postNameWs", "");
        out << term->stringPropOptional("syntax:properties", "");

        out << "(";

        bool first = true;
        int numInputs = function_t::num_inputs(term);
        for (int i=0; i < numInputs; i++) {
            std::string name = function_t::get_input_name(term, i);

            if (name == "#state")
                continue;

            Term* input = function_t::get_input_placeholder(term, i);

            if (input->boolPropOptional("state", false))
                out << "state ";

            if (!first) out << ", ";
            first = false;
            out << function_t::get_input_type(term, i)->name;

            if (name != "" && name[0] != '#')
                out << " " << name;
        }

        if (function_t::get_variable_args(term))
            out << "...";

        out << ")";

        if (function_get_output_type(term, 0) != VOID_TYPE) {
            out << term->stringPropOptional("syntax:whitespacePreColon", "");
            out << "->";
            out << term->stringPropOptional("syntax:whitespacePostColon", " ");
            out << function_get_output_type(term, 0)->name;
        }

        return out.str();
    }

    void format_header_source(StyledSource* source, Term* term)
    {
        append_phrase(source, term->name, term, phrase_type::TERM_NAME);

        append_phrase(source, term->stringPropOptional("syntax:postNameWs", ""),
                term, token::WHITESPACE);
        append_phrase(source, term->stringPropOptional("syntax:properties", ""),
                term, phrase_type::UNDEFINED);

        append_phrase(source, "(", term, token::LPAREN);

        bool first = true;
        int numInputs = function_t::num_inputs(term);
        for (int i=0; i < numInputs; i++) {
            std::string name = function_t::get_input_name(term, i);

            if (name == "#state")
                continue;

            Term* input = function_t::get_input_placeholder(term, i);

            if (input->boolPropOptional("state", false))
                append_phrase(source, "state ", term, phrase_type::UNDEFINED);

            if (!first)
                append_phrase(source, ", ", term, phrase_type::UNDEFINED);
            first = false;

            if (!first)
                append_phrase(source, function_t::get_input_type(term, i)->name,
                        term, phrase_type::TYPE_NAME);

            if (name != "" && name[0] != '#') {
                append_phrase(source, " ", term, token::WHITESPACE);
                append_phrase(source, name, term, phrase_type::UNDEFINED);
            }

            if (function_can_rebind_input(term, i)) {
                append_phrase(source, " ", term, token::WHITESPACE);
                append_phrase(source, ":out", term, phrase_type::UNDEFINED);
            }
        }

        if (function_t::get_variable_args(term))
            append_phrase(source, "...", term, phrase_type::UNDEFINED);

        append_phrase(source, ")", term, token::LPAREN);

        if (function_get_output_type(term, 0) != VOID_TYPE) {
            append_phrase(source, term->stringPropOptional("syntax:whitespacePreColon", ""),
                    term, token::WHITESPACE);
            append_phrase(source, "->", term, phrase_type::UNDEFINED);
            append_phrase(source, term->stringPropOptional("syntax:whitespacePostColon", ""),
                    term, token::WHITESPACE);
            append_phrase(source, function_get_output_type(term, 0)->name,
                    term, phrase_type::TYPE_NAME);
        }
    }

    std::string get_documentation(Term* func)
    {
        // A function can optionally have a documentation string. If present,
        // it will be the first thing defined in the function, and it'll be
        // anonymous and be a statement.
        int expected_index = function_t::num_inputs(func) + 1;
        Branch& contents = nested_contents(func);

        if (expected_index >= contents.length()) return "";
        Term* possibleDocString = contents[expected_index];
        if (possibleDocString->name != "") return "";
        if (!is_string(possibleDocString)) return "";
        if (!is_statement(possibleDocString)) return "";
        if (!is_value(possibleDocString)) return "";
        return as_string(possibleDocString);
    }

    bool check_invariants(Term* func, std::string* failureMessage)
    {
        if (!is_subroutine(func))
            return true;

        //Branch& contents = nested_contents(func);

        // There was once stuff here

        return true;
    }

    void setup_type(Type* type)
    {
        type->name = "Function";
        type->formatSource = subroutine_f::format_source;
        type->checkInvariants = function_t::check_invariants;
        type->storageType = STORAGE_TYPE_REF;
    }

    std::string const& get_name(Term* function)
    {
        return get_function_attrs(function)->name;
    }

    bool get_variable_args(Term* function)
    {
        if (!is_function(function)) return true;
        FunctionAttrs* attrs = get_function_attrs(function);
        if (attrs == NULL) return true;
        return attrs->variableArgs;
    }

    int num_inputs(Term* function)
    {
        Branch& contents = nested_contents(function);
        int i = 1;

        while (i < contents.length()
                && contents[i] != NULL
                && contents[i]->function == INPUT_PLACEHOLDER_FUNC)
            i++;
        return i - 1;
    }

    Term* get_input_placeholder(Term* func, int index)
    {
        index += 1;
        if (index >= nested_contents(func).length())
            return NULL;
        return nested_contents(func)[index];
    }

    std::string const& get_input_name(Term* func, int index)
    {
        return function_t::get_input_placeholder(func, index)->name;
    }

    bool get_input_meta(Term* func, int index)
    {
        if (!is_function(func)) return false;
        Term* placeholder = function_t::get_input_placeholder(func, index);
        if (placeholder == NULL)
            return false;
        return placeholder->boolPropOptional("meta", false);
    }

    bool get_input_optional(Term* func, int index)
    {
        if (!is_function(func))
            return true;
        Term* placeholder = function_t::get_input_placeholder(func, index);
        if (placeholder == NULL)
            return false;
        return placeholder->boolPropOptional("optional", false);
    }

    Term* get_input_type(Term* func, int index)
    {
        if (!is_function(func))
            return ANY_TYPE;

        if (function_t::get_variable_args(func))
            index = 0;

        return function_t::get_input_placeholder(func, index)->type;
    }
    bool is_state_input(Term* function, int index)
    {
        if (!is_function(function))
            return false;
        return get_input_placeholder(function,index)->boolPropOptional("state", false);
    }
    EvaluateFunc& get_evaluate(Term* func)
    {
        return get_function_attrs(func)->evaluate;
    }
    SpecializeTypeFunc& get_specialize_type(Term* func)
    {
        return get_function_attrs(func)->specializeType;
    }
} // namespace function_t

bool is_function(Term* term)
{
    if (term == NULL)
        return false;
    return term->type == FUNCTION_TYPE;
}

bool is_function_attrs(Term* term)
{
    return term->type == FUNCTION_ATTRS_TYPE;
}

FunctionAttrs& as_function_attrs(Term* term)
{
    ca_assert(is_function_attrs(term));
    ca_assert(get_pointer(term) != NULL);
    return *((FunctionAttrs*) get_pointer(term));
}

Branch& function_contents(Term* func)
{
    return nested_contents(func);
}

FunctionAttrs* get_function_attrs(Term* func)
{
    if (func == NULL)
        return NULL;
    if (nested_contents(func).length() == 0)
        return NULL;
    if (nested_contents(func)[0]->type != FUNCTION_ATTRS_TYPE)
        return NULL;
    return &as_function_attrs(nested_contents(func)[0]);
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

    Term* attributesTerm = create_value(nested_contents(func), FUNCTION_ATTRS_TYPE, "#attributes");
    hide_from_source(attributesTerm);

    // Setup the term's global value to point back to the term, so that the function
    // can be passed as a value.
    set_function_pointer(func, func);
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

    for (int i=0; i < function_t::num_inputs(func); i++) {
        Term* input = function_t::get_input_placeholder(func, i);
        if (input->boolPropOptional("output", false)) {
            attrs->outputCount++;
            set_type(attrs->outputTypes.append(), get_output_type(input));
        }
    }
}

bool is_callable(Term* term)
{
    return (term->type == FUNCTION_TYPE
            || term->type == TYPE_TYPE);
}

bool inputs_statically_fit_function(Term* func, TermList const& inputs)
{
    bool varArgs = function_t::get_variable_args(func);

    // Fail if wrong # of inputs
    if (!varArgs && (function_t::num_inputs(func) != inputs.length()))
        return false;

    for (int i=0; i < inputs.length(); i++) {
        Type* type = unbox_type(function_t::get_input_type(func, i));
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
    bool varArgs = function_t::get_variable_args(func);

    // Fail if wrong # of inputs
    if (!varArgs && (function_t::num_inputs(func) != inputs.length()))
        return false;

    for (int i=0; i < inputs.length(); i++) {
        Type* type = unbox_type(function_t::get_input_type(func, i));
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
    bool varArgs = function_t::get_variable_args(func);

    // Fail if wrong # of inputs
    if (!varArgs && (function_t::num_inputs(func) != list->length()))
        return false;

    for (int i=0; i < list->length(); i++) {
        Type* type = unbox_type(function_t::get_input_type(func, i));
        TaggedValue* value = list->get(i);
        if (value == NULL)
            continue;
        if (!cast_possible(value, type))
            return false;
    }
    return true;
}

Term* create_overloaded_function(Branch& branch, std::string const& name,
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
    Type* outputType = function_get_output_type(function, 0);
    if (function_t::get_specialize_type(function) != NULL)
        outputType = function_t::get_specialize_type(function)(call);
    if (outputType == NULL)
        outputType = &ANY_T;
    return outputType;
}

bool function_can_rebind_input(Term* function, int index)
{
    if (function_t::get_variable_args(function))
        index = 0;

    Term* input = function_t::get_input_placeholder(function, index);
    if (input == NULL)
        return false;
    return input->boolPropOptional("output", false);
}

bool function_implicitly_rebinds_input(Term* function, int index)
{
    Term* input = function_t::get_input_placeholder(function, index);
    if (input == NULL)
        return false;
    return input->boolPropOptional("use-as-output", false);
}

bool function_call_rebinds_input(Term* term, int index)
{
    return get_input_syntax_hint_optional(term, index, "rebindInput", "") == "t";
}

Type* function_get_input_type(Term* function, int index)
{
    if (function_t::get_variable_args(function))
        index = 0;
    return function_t::get_input_placeholder(function, index)->type;
}

Type* function_get_output_type(Term* function, int index)
{
    FunctionAttrs* attrs = get_function_attrs(function);

    if (attrs == NULL)
        return ANY_TYPE;

    // Temporary special case
    if (index > 0 && (function == IF_BLOCK_FUNC || function == FOR_FUNC))
        return ANY_TYPE;

    ca_assert(index < attrs->outputTypes.length());

    return as_type(attrs->outputTypes[index]);
}
TaggedValue* function_get_parameters(Term* function)
{
    FunctionAttrs* attrs = get_function_attrs(function);
    if (attrs == NULL)
        return NULL;
    return &attrs->parameter;
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
    for (int inputIndex=0; inputIndex < function_t::num_inputs(function); inputIndex++) {
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

bool is_native_function(Term* func)
{
    if (!is_function(func))
        return false;
    return function_t::get_evaluate(func) != evaluate_subroutine;
}

void function_set_evaluate_func(Term* function, EvaluateFunc evaluateFunc)
{
    get_function_attrs(function)->evaluate = evaluateFunc;
    on_evaluate_function_changed(function);
}

} // namespace circa
