// Copyright 2008 Andrew Fischer

#include "circa.h"

namespace circa {

Function::Function()
  : outputType(NULL),
    hiddenStateType(NULL),
    variableArgs(false),
    evaluate(NULL),
    specializeType(NULL),
    toSourceString(NULL)
{
}

Term*
Function::inputType(int index)
{
    if (variableArgs)
        return inputTypes[0];
    else
        return inputTypes[index];
}

std::string const&
Function::inputName(int index)
{
    return getInputProperties(index).name;
}

int Function::numInputs()
{
    return inputTypes.length();
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
    assert(variableArgs || index < inputTypes.length());

    // check to grow inputProperties
    while ((index+1) > (int) inputProperties.size()) {
        inputProperties.push_back(InputProperties());
    }

    return inputProperties[index];
}

void Function::setInputMeta(int index, bool value)
{
    getInputProperties(index).meta = value;
}

void Function::setInputModified(int index, bool value)
{
    getInputProperties(index).modified = value;
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
        copy_field(outputType);
        copy_field(variableArgs);
        copy_field(name);
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
        func.outputType = map.getRemapped(func.outputType);
    }
    std::string to_string(Term* term)
    {
        return "<Function " + as_function(term).name + ">";
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

Term* function_get_input_type(Term* func, int index)
{
    Function& data = get_function_data(func);

    if (data.variableArgs)
        return data.inputType(0);
    else
        return data.inputType(index);
}

bool is_callable(Term* term)
{
    return (term->type == FUNCTION_TYPE)
        || (term->type == SUBROUTINE_TYPE)
        || (term->type == OVERLOADED_FUNCTION_TYPE);
}

bool inputs_fit_function(Term* func, RefList const& inputs)
{
    Function& data = get_function_data(func);

    // Fail if wrong # of inputs
    if (!data.variableArgs && (data.numInputs() != inputs.length()))
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
