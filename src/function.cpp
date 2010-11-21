// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"
#include "debug_valid_objects.h"

namespace circa {

FunctionAttrs::FunctionAttrs()
  : variableArgs(false),
    throws(false),
    currentlyEvaluating(false),
    evaluate(NULL),
    specializeType(NULL),
    formatSource(NULL),
    checkInvariants(NULL),
    staticTypeQuery(NULL),
    writeBytecode(NULL),
    postInputChange(NULL),
    getRegisterCount(NULL),
    assignRegisters(NULL)
{
    debug_register_valid_object(this, FUNCTION_ATTRS_OBJECT);
}

FunctionAttrs::~FunctionAttrs()
{
    debug_unregister_valid_object(this);
}

namespace function_attrs_t {

    void initialize(Type* type, TaggedValue* value)
    {
        FunctionAttrs* attrs = new FunctionAttrs();
        set_pointer(value, type, attrs);
    }

    void release(TaggedValue* value)
    {
        delete (FunctionAttrs*) get_pointer(value);
    }

    void copy(TaggedValue* source, TaggedValue* dest)
    {
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

        if (function_t::get_output_type(term) != VOID_TYPE) {
            out << term->stringPropOptional("syntax:whitespacePreColon", "");
            out << "->";
            out << term->stringPropOptional("syntax:whitespacePostColon", " ");
            out << function_t::get_output_type(term)->name;
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
        }

        if (function_t::get_variable_args(term))
            append_phrase(source, "...", term, phrase_type::UNDEFINED);

        append_phrase(source, ")", term, token::LPAREN);

        if (function_t::get_output_type(term) != VOID_TYPE) {
            append_phrase(source, term->stringPropOptional("syntax:whitespacePreColon", ""),
                    term, token::WHITESPACE);
            append_phrase(source, "->", term, phrase_type::UNDEFINED);
            append_phrase(source, term->stringPropOptional("syntax:whitespacePostColon", ""),
                    term, token::WHITESPACE);
            append_phrase(source, function_t::get_output_type(term)->name,
                    term, phrase_type::TYPE_NAME);
        }
    }

    std::string get_documentation(Term* func)
    {
        // A function can optionally have a documentation string. If present,
        // it will be the first thing defined in the function, and it'll be
        // anonymous and be a statement.
        int expected_index = function_t::num_inputs(func) + 1;
        Branch& contents = func->nestedContents;

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

        //Branch& contents = func->nestedContents;

        // There was once stuff here

        return true;
    }

    FunctionAttrs& get_attrs(Term* function)
    {
        ca_assert(function->nestedContents.length() > 0);
        ca_assert(function->nestedContents[0]->type == FUNCTION_ATTRS_TYPE);
        FunctionAttrs* attrs= &as_function_attrs(function->nestedContents[0]);
        debug_assert_valid_object(attrs, FUNCTION_ATTRS_OBJECT);
        return *attrs;
    }

    std::string const& get_name(Term* function)
    {
        return get_attrs(function).name;
    }
    void set_name(Term* function, std::string const& name)
    {
        get_attrs(function).name = name;
    }

    Term* get_output_type(Term* function)
    {
        if (!is_function(function)) return ANY_TYPE;
        return get_attrs(function).outputType;
    }
    void set_output_type(Term* function, Term* type)
    {
        get_attrs(function).outputType = type;
    }

    Ref& get_inline_state_type(Term* function)
    {
        return get_attrs(function).implicitStateType;
    }

    bool get_variable_args(Term* function)
    {
        if (!is_function(function)) return true;
        return get_attrs(function).variableArgs;
    }

    void set_variable_args(Term* function, bool value)
    {
        get_attrs(function).variableArgs = value;
    }

    int num_inputs(Term* function)
    {
        Branch& contents = function->nestedContents;
        int i = 1;

        while (i < contents.length()
                && contents[i] != NULL
                && contents[i]->function == INPUT_PLACEHOLDER_FUNC)
            i++;
        return i - 1;
    }

    Term* get_input_placeholder(Term* func, int index)
    {
        return func->nestedContents[index + 1];
    }

    std::string const& get_input_name(Term* func, int index)
    {
        return function_t::get_input_placeholder(func, index)->name;
    }

    bool get_input_meta(Term* func, int index)
    {
        if (!is_function(func)) return false;
        return function_t::get_input_placeholder(func, index)->boolPropOptional("meta", false);
    }

    void set_input_meta(Term* func, int index, bool value)
    {
        function_t::get_input_placeholder(func, index)->setBoolProp("meta", value);
    }

    bool get_input_optional(Term* func, int index)
    {
        if (!is_function(func))
            return true;
        return function_t::get_input_placeholder(func, index)->boolPropOptional("optional", false);
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
        return get_attrs(func).evaluate;
    }
    SpecializeTypeFunc& get_specialize_type(Term* func)
    {
        return get_attrs(func).specializeType;
    }
    std::string const& get_exposed_name_path(Term* func)
    {
        return get_attrs(func).exposedNamePath;
    }
    void set_exposed_name_path(Term* func, std::string const& value)
    {
        get_attrs(func).exposedNamePath = value;
    }
    Ref& get_feedback_func(Term* func)
    {
        return get_attrs(func).feedbackFunc;
    }
    TaggedValue* get_parameters(Term* func)
    {
        return &get_attrs(func).parameter;
    }
    std::string const& get_description(Term* func)
    {
        return get_attrs(func).description;
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
    return func->nestedContents;
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

    Term* attributesTerm = create_value(func->nestedContents, FUNCTION_ATTRS_TYPE, "#attributes");
    hide_from_source(attributesTerm);
}

bool is_callable(Term* term)
{
    return (term->type == FUNCTION_TYPE
            || term->type == TYPE_TYPE);
}

bool inputs_statically_fit_function(Term* func, RefList const& inputs)
{
    bool varArgs = function_t::get_variable_args(func);

    // Fail if wrong # of inputs
    if (!varArgs && (function_t::num_inputs(func) != inputs.length()))
        return false;

    for (int i=0; i < inputs.length(); i++) {
        Type* type = type_contents(function_t::get_input_type(func, i));
        Term* input = inputs[i];
        if (input == NULL)
            continue;

        bool alwaysSatisfiesType = term_output_always_satisfies_type(input, type);
        if (!alwaysSatisfiesType)
            return false;
    }

    return true;
}

bool inputs_fit_function_dynamic(Term* func, RefList const& inputs)
{
    bool varArgs = function_t::get_variable_args(func);

    // Fail if wrong # of inputs
    if (!varArgs && (function_t::num_inputs(func) != inputs.length()))
        return false;

    for (int i=0; i < inputs.length(); i++) {
        Type* type = type_contents(function_t::get_input_type(func, i));
        TaggedValue* value = inputs[i];
        if (value == NULL)
            continue;
        if (!value_fits_type(value, type))
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
        Type* type = type_contents(function_t::get_input_type(func, i));
        TaggedValue* value = list->get(i);
        if (value == NULL)
            continue;
        if (!value_fits_type(value, type))
            return false;
    }
    return true;
}

Term* create_overloaded_function(Branch& branch, std::string const& name,
        RefList const& overloads)
{
    return overloaded_function::create_overloaded_function(branch, name, overloads);
}

Term* function_get_specialized_output_type(Term* function, Term* call)
{
    if (!is_function(function))
        return ANY_TYPE;
    Term* outputType = function_t::get_output_type(function);
    if (function_t::get_specialize_type(function) != NULL)
        outputType = function_t::get_specialize_type(function)(call);
    return outputType;
}

void function_set_use_input_as_output(Term* function, int index, bool value)
{
    Term* placeholder = function_t::get_input_placeholder(function, index);
    placeholder->setBoolProp("use-as-output", value);
}

bool is_native_function(Term* func)
{
    if (!is_function(func))
        return false;
    return function_t::get_evaluate(func) != subroutine_t::evaluate;
}

} // namespace circa
