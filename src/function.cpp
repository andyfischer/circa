// Copyright 2008 Paul Hodge

#include "circa.h"

namespace circa {

Function::Function()
  : _outputType(NULL),
    _hiddenStateType(NULL),
    _variableArgs(false),
    evaluate(NULL),
    specializeType(NULL),
    toSourceString(NULL)
{
}

void Function::appendInput(Term* type, std::string const& name)
{
    inputTypes.append(type);
    getInputProperties(inputTypes.length()-1).name = name;
}

void Function::prependInput(Term* type, std::string const& name)
{
    InputProperties props;
    props.name = name;
    inputTypes.prepend(type);
    inputProperties.insert(inputProperties.begin(), props);
}

Function::InputProperties&
Function::getInputProperties(int index)
{
    assert(_variableArgs || index < inputTypes.length());

    // check to grow inputProperties
    while ((index+1) > (int) inputProperties.size()) {
        inputProperties.push_back(InputProperties());
    }

    return inputProperties[index];
}



namespace function_t {
    void assign(Term* sourceTerm, Term* destTerm)
    {
        assert(is_function(sourceTerm));
        assert(is_function(destTerm));

        Function &source = as_function(sourceTerm);
        Function &dest = as_function(destTerm);

        dest = Function();

        #define copy_field(f) dest.f = source.f

        copy_field(inputTypes);
        copy_field(inputProperties);
        copy_field(_outputType);
        copy_field(_hiddenStateType);
        copy_field(_variableArgs);
        copy_field(_name);
        copy_field(evaluate);
        copy_field(specializeType);
        copy_field(toSourceString);
        copy_field(feedbackFunc);

        #undef copy_field
    }

    void remapPointers(Term* term, ReferenceMap const& map)
    {
        Function &func = as_function(term);
        func.inputTypes.remapPointers(map);
        function_get_output_type(term) = 
            map.getRemapped(function_get_output_type(term));
    }
    std::string to_string(Term* term)
    {
        return "<Function " + function_get_name(term) + ">";
    }
}

bool is_function(Term* term)
{
    return term->type == FUNCTION_TYPE;
}

Function& as_function(Term* term)
{
    assert_type(term, FUNCTION_TYPE);
    alloc_value(term);
    return *((Function*) term->value);
}

std::string get_placeholder_name_for_index(int index)
{
    std::stringstream sstream;
    sstream << INPUT_PLACEHOLDER_PREFIX << index;
    return sstream.str();
}

std::string& function_get_name(Term* function)
{
    return get_function_data(function)._name;
}

Ref& function_get_output_type(Term* function)
{
    return get_function_data(function)._outputType;
}

Ref& function_get_hidden_state_type(Term* function)
{
    return get_function_data(function)._hiddenStateType;
}

bool& function_get_variable_args(Term* function)
{
    return get_function_data(function)._variableArgs;
}

int function_num_inputs(Term* function)
{
    return get_function_data(function).inputProperties.size();
}

std::string const& function_get_input_name(Term* function, int index)
{
    return get_function_data(function).getInputProperties(index).name;
}

bool& function_get_input_meta(Term* function, int index)
{
    return get_function_data(function).getInputProperties(index).meta;
}

bool& function_get_input_modified(Term* function, int index)
{
    return get_function_data(function).getInputProperties(index).modified;
}

Term* function_get_input_type(Term* func, int index)
{
    Function& data = get_function_data(func);

    if (function_get_variable_args(func))
        return data.inputTypes[0];
    else
        return data.inputTypes[index];
}

Function::EvaluateFunc& function_get_evaluate(Term* function)
{
    return get_function_data(function).evaluate;
}

Function::SpecializeTypeFunc& function_get_specialize_type(Term* function)
{
    return get_function_data(function).specializeType;
}

Function::ToSourceString& function_get_to_source_string(Term* function)
{
    return get_function_data(function).toSourceString;
}

bool is_callable(Term* term)
{
    return (term->type == FUNCTION_TYPE)
        || (term->type == SUBROUTINE_TYPE)
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

Term* create_overloaded_function(Branch* branch, std::string const& name)
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
            if (is_ref(func)) func = deref(func);
            if (inputs_fit_function(func, inputs))
                return func;
        }

        return UNKNOWN_FUNCTION;
    } else {
        return func;
    }
}

} // namespace circa
