// Copyright (c) 2007-2010 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {

namespace function_attrs_t {

    void initialize(Type* type, TaggedValue* value)
    {
        set_pointer(value, type, new FunctionAttrs());
    }

    void destroy(Type* type, TaggedValue* value)
    {
        delete (FunctionAttrs*) get_pointer(value);
    }

    void assign(TaggedValue* source, TaggedValue* dest)
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

    std::string get_documentation(Term* term)
    {
        // A function can optionally have a documentation string. If present,
        // it will be the first thing defined in the function, and it'll be
        // anonymous and be a statement.
        int expected_index = function_t::num_inputs(term) + 1;
        Branch& contents = as_branch(term);

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

        Branch& contents = as_branch(func);

        // If the subroutine has an #out term, then it must be the last one
        if (contents.contains("#out") && contents[contents.length()-1]->name != "#out") {

            if (failureMessage != NULL)
                *failureMessage = "#out is bound, but the last term isn't named #out";
            return false;
        }

        return true;
    }

    std::string const& get_name(Term* function)
    {
        return function->asBranch()[0]->asBranch()[0]->asString();
    }
    void set_name(Term* function, std::string const& name)
    {
        set_str(function->asBranch()[0]->asBranch()[0], name);
    }

    Term* get_output_type(Term* function)
    {
        Branch& contents = as_branch(function);
        Term* last_term = contents[contents.length()-1];
        if (last_term->name == "#out")
            return last_term->type;
        else
            return VOID_TYPE;
    }

    Ref& get_hidden_state_type(Term* function)
    {
        return function->asBranch()[0]->asBranch()[1]->asRef();
    }

    bool get_variable_args(Term* function)
    {
        return function->asBranch()[0]->asBranch()[2]->asBool();
    }

    void set_variable_args(Term* function, bool value)
    {
        set_bool(function->asBranch()[0]->asBranch()[2], value);
    }

    int num_inputs(Term* function)
    {
        Branch& contents = as_branch(function);
        int i = 1;

        while (i < contents.length()
                && contents[i] != NULL
                && contents[i]->function == INPUT_PLACEHOLDER_FUNC)
            i++;
        return i - 1;
    }

    Term* get_input_placeholder(Term* func, int index)
    {
        return as_branch(func)[index + 1];
    }

    std::string const& get_input_name(Term* func, int index)
    {
        return function_t::get_input_placeholder(func, index)->name;
    }

    bool get_input_meta(Term* func, int index)
    {
        return function_t::get_input_placeholder(func, index)->boolPropOptional("meta", false);
    }

    void set_input_meta(Term* func, int index, bool value)
    {
        function_t::get_input_placeholder(func, index)->setBoolProp("meta", value);
    }

    bool get_input_modified(Term* func, int index)
    {
        return function_t::get_input_placeholder(func, index)->boolPropOptional("modified", false);
    }

    Term* get_input_type(Term* func, int index)
    {
        if (function_t::get_variable_args(func))
            index = 0;

        return function_t::get_input_placeholder(func, index)->type;
    }
    EvaluateFunc& get_evaluate(Term* func)
    {
        return as_evaluate_thunk(func->asBranch()[0]->asBranch()[3]);
    }
    SpecializeTypeFunc& get_specialize_type(Term* func)
    {
        return as_specialize_type_thunk(func->asBranch()[0]->asBranch()[4]);
    }
    ToSourceStringFunc& get_to_source_string(Term* func)
    {
        return as_to_source_string_thunk(func->asBranch()[0]->asBranch()[5]);
    }
    std::string const& get_exposed_name_path(Term* func)
    {
        return as_string(func->asBranch()[0]->asBranch()[6]);
    }
    void set_exposed_name_path(Term* func, std::string const& value)
    {
        set_str(func->asBranch()[0]->asBranch()[6], value);
    }
    Ref& get_feedback_func(Term* func)
    {
        return as_ref(func->asBranch()[0]->asBranch()[7]);
    }
    Branch& get_parameters(Term* func)
    {
        return as_branch(func->asBranch()[0]->asBranch()[8]);
    }
    std::string const& get_description(Term* func)
    {
        return as_string(func->asBranch()[0]->asBranch()[9]);
    }
} // namespace function_t

bool is_function(Term* term)
{
    return term->type == FUNCTION_TYPE;
}

std::string get_placeholder_name_for_index(int index)
{
    std::stringstream sstream;
    sstream << "#input_" << index;
    return sstream.str();
}

void initialize_function_prototype(Branch& contents)
{
    /* A function is a branch with the following structures:
      {
        [0] #attributes {
          [0] string name
          [1] ref hidden_state_type
          [2] bool variable_args
          [3] EvaluateFunc native_evaluate
          [4] SpecializeTypeFunc native_specialize_type
          [5] ToSourceStringFunc native_to_source_string
          [6] string exposed_name_path
          [7] ref feedback_func
          [8] List parameters
          [9] string description
        }
        [1..num_inputs] input terms
            .. each might have bool property 'modified' or 'meta'
        [...] body
        [n-1] output term, type is significant
      }
    */

    /* Side note: currently it is kind of awkward to have the last term define
     * the output type. This causes ugliness in a few places. For one, it means
     * that we can't check a function's output type while we are still building it.
     * There are also some backflips required to make sure that the #out term is
     * last. Probably should revisit this.
     */

    Term* attributesTerm = create_value(contents, BRANCH_TYPE, "#attributes");
    set_source_hidden(attributesTerm, true);
    Branch& attributes = as_branch(attributesTerm);
    create_string(attributes, "", "name");
    create_ref(attributes, NULL, "hidden_state_type");
    create_bool(attributes, false, "variable_args");
    create_value(attributes, EVALUATE_THUNK_TYPE, "native_evaluate");
    create_value(attributes, SPECIALIZE_THUNK_TYPE, "native_specialize");
    create_value(attributes, TO_STRING_THUNK_TYPE, "native_to_string");
    create_string(attributes, "", "exposed_name_path");
    create_ref(attributes, NULL, "feedback_func");
    create_value(attributes, LIST_TYPE, "parameters");
    create_value(attributes, STRING_TYPE, "description");
}

bool is_callable(Term* term)
{
    return (term->type == FUNCTION_TYPE
            || term->type == OVERLOADED_FUNCTION_TYPE
            || term->type == TYPE_TYPE);
}

bool inputs_fit_function(Term* func, RefList const& inputs)
{
    bool varArgs = function_t::get_variable_args(func);

    // Fail if wrong # of inputs
    if (!varArgs && (function_t::num_inputs(func) != inputs.length()))
        return false;

    for (int i=0; i < inputs.length(); i++) {
        Term* type = function_t::get_input_type(func, i);
        if (inputs[i] == NULL)
            continue;
        if (!value_fits_type(inputs[i], type))
            return false;
    }

    return true;
}

Term* create_overloaded_function(Branch& branch, std::string const& name)
{
    return create_value(branch, OVERLOADED_FUNCTION_TYPE, name);
}

Term* function_get_specialized_output_type(Term* function, Term* call)
{
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
    return function_t::get_evaluate(func) != subroutine_t::evaluate;
}

Term* specialize_function(Term* func, RefList const& inputs)
{
    if (func->type == OVERLOADED_FUNCTION_TYPE) {
        // Find a match among possible overloads
        Branch& overloads = as_branch(func);
        for (int i=0; i < overloads.length(); i++) {
            Term* overload = overloads[i];
            if (is_ref(overload)) overload = as_ref(overload);
            if (inputs_fit_function(overload, inputs))
                return overload;
        }

        return UNKNOWN_FUNCTION;
    }

    if (inputs_fit_function(func, inputs))
        return func;

    return func;
}

} // namespace circa
