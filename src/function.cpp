// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {

namespace function_t {
    std::string to_string(Term* term)
    {
        return "<Function " + function_get_name(term) + ">";
    }

    bool check_invariants(Term* func, std::stringstream* output)
    {
        if (!is_subroutine(func))
            return true;

        Branch& contents = as_branch(func);

        // If the subroutine has an #out term, then it must be the last one
        if (contents.contains(OUTPUT_PLACEHOLDER_NAME)
                && contents[contents.length()-1]->name != OUTPUT_PLACEHOLDER_NAME) {

            if (output != NULL)
                *output << "#out is bound, but the last term isn't named #out";
            return false;
        }

        return true;
    }
}

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
        [0] attributes {
          [0] string name
          [1] Ref hidden_state_type
          [2] bool variable_args
          [3] EvaluateFunc native_evaluate
          [4] SpecializeTypeFunc native_specialize_type
          [5] ToSourceStringFunc native_to_source_string
          [6] Ref feedback_func
          [7] List parameters
        }
        [1..num_inputs] input terms
            .. each might have bool property 'modified' or 'meta'
        [...] body
        [n-1] output term, type is significant
      }
    */

    Branch& contents = as_branch(term);

    Term* attributesTerm = create_value(contents, BRANCH_TYPE, "attributes");
    source_set_hidden(attributesTerm, true);
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

std::string& function_get_name(Term* function)
{
    return function->field(0)->field(0)->asString();
}

Ref& function_get_output_type(Term* function)
{
    Branch& contents = as_branch(function);
    return contents[contents.length()-1]->type;
}

Ref& function_get_hidden_state_type(Term* function)
{
    return function->field(0)->field(1)->asRef();
}

bool& function_get_variable_args(Term* function)
{
    return function->field(0)->field(2)->asBool();
}

int function_num_inputs(Term* function)
{
    Branch& contents = as_branch(function);
    int i = 1;

    while (i < contents.length()
            && contents[i] != NULL
            && contents[i]->function == INPUT_PLACEHOLDER_FUNC)
        i++;
    return i - 1;
}

Term*& function_get_input_placeholder(Term* func, int index)
{
    return as_branch(func)[index + 1];
}

std::string const& function_get_input_name(Term* func, int index)
{
    return function_get_input_placeholder(func, index)->name;
}

bool function_get_input_meta(Term* func, int index)
{
    return function_get_input_placeholder(func, index)->boolPropOptional("meta", false);
}

void function_set_input_meta(Term* func, int index, bool value)
{
    function_get_input_placeholder(func, index)->boolProp("meta") = value;
}

bool function_get_input_modified(Term* func, int index)
{
    return function_get_input_placeholder(func, index)->boolPropOptional("modified", false);
}

Term* function_get_input_type(Term* func, int index)
{
    if (function_get_variable_args(func))
        index = 0;

    return function_get_input_placeholder(func, index)->type;
}

EvaluateFunc& function_get_evaluate(Term* func)
{
    return as_evaluate_thunk(func->field(0)->field(3));
}

SpecializeTypeFunc& function_get_specialize_type(Term* func)
{
    return as_specialize_type_thunk(func->field(0)->field(4));
}

ToSourceStringFunc& function_get_to_source_string(Term* func)
{
    return as_to_source_string_thunk(func->field(0)->field(5));
}

Ref& function_get_feedback_func(Term* func)
{
    return as_ref(func->field(0)->field(6));
}

Branch& function_get_parameters(Term* func)
{
    return as_branch(func->field(0)->field(7));
}

bool is_callable(Term* term)
{
    return (term->type == FUNCTION_TYPE)
        || (term->type == OVERLOADED_FUNCTION_TYPE);
}

bool inputs_fit_function(Term* func, RefList const& inputs)
{
    bool varArgs = function_get_variable_args(func);

    // Fail if wrong # of inputs
    if (!varArgs && (function_num_inputs(func) != inputs.length()))
        return false;

    for (int i=0; i < inputs.length(); i++) {
        Term* type = function_get_input_type(func, i);
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
