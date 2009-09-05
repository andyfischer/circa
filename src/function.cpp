// Copyright (c) 2007-2009 Paul Hodge. All rights reserved.

#include "circa.h"

namespace circa {

namespace function_t {
    std::string to_string(Term* term)
    {
        return "<Function " + function_t::get_name(term) + ">";
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

    std::string& get_name(Term* function)
    {
        return function->asBranch()[0]->asBranch()[0]->asString();
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

    bool& get_variable_args(Term* function)
    {
        return function->asBranch()[0]->asBranch()[2]->asBool();
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
        function_t::get_input_placeholder(func, index)->boolProp("meta") = value;
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

    Ref& get_feedback_func(Term* func)
    {
        return as_ref(func->asBranch()[0]->asBranch()[6]);
    }

    Branch& get_parameters(Term* func)
    {
        return as_branch(func->asBranch()[0]->asBranch()[7]);
    }
} // namespace function_t

bool is_function(Term* term)
{
    return term->type == FUNCTION_TYPE;
}

std::string get_placeholder_name_for_index(int index)
{
    std::stringstream sstream;
    sstream << INPUT_PLACEHOLDER_PREFIX << index;
    return sstream.str();
}

void initialize_function_data(Term* term)
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
          [6] ref feedback_func
          [7] List parameters
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

    Branch& contents = as_branch(term);

    Term* attributesTerm = create_value(contents, BRANCH_TYPE, "#attributes");
    set_source_hidden(attributesTerm, true);
    Branch& attributes = as_branch(attributesTerm);
    string_value(attributes, "", "name");
    create_ref(attributes, NULL, "hidden_state_type");
    bool_value(attributes, false, "variable_args");
    create_value(attributes, EVALUATE_THUNK_TYPE, "native_evaluate");
    create_value(attributes, SPECIALIZE_THUNK_TYPE, "native_specialize");
    create_value(attributes, TO_STRING_THUNK_TYPE, "native_to_string");
    create_ref(attributes, NULL, "feedback_func");
    create_value(attributes, LIST_TYPE, "parameters");
}

bool is_callable(Term* term)
{
    return (term->type == FUNCTION_TYPE)
        || (term->type == OVERLOADED_FUNCTION_TYPE)
        || (term->type == TYPE_TYPE);
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

Term* specialize_function(Term* func, RefList const& inputs)
{
    if (func->type == OVERLOADED_FUNCTION_TYPE) {
        // Find a match among possible overloads
        Branch& overloads = as_branch(func);
        for (int i=0; i < overloads.length(); i++) {
            Term* func = overloads[i];
            if (is_ref(func)) func = as_ref(func);
            if (inputs_fit_function(func, inputs))
                return func;
        }

        return UNKNOWN_FUNCTION;
    } else {
        return func;
    }
}

} // namespace circa
