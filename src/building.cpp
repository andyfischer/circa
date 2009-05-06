// Copyright 2009 Andrew Fischer

#include "circa.h"

namespace circa {

Term* create_term(Branch* branch, Term* function, RefList const& inputs)
{
    assert_good_pointer(function);

    if (!is_function(function)) {
        assert(false);
        throw std::runtime_error("in create_term, 2nd arg to create_term must be a function");
    }

    Term* term = new Term();

    if (branch != NULL)
        branch->append(term);

    term->function = function;
    term->needsUpdate = true;

    Function& functionData = as_function(function);

    // Initialize inputs
    for (unsigned int i=0; i < inputs.count(); i++)
        set_input(term, i, inputs[i]);

    Term* outputType = functionData.outputType;

    // Check if this function has a specializeType function
    // Side note: maybe we should do this step later. Doing it here means that we can only
    // specialize on inputs, but it might be cool to specialize on state too.
    if (functionData.specializeType != NULL) {
        outputType = functionData.specializeType(term);
        if (outputType == NULL)
            throw std::runtime_error("result of specializeType is NULL");
    }

    if (outputType == NULL)
        throw std::runtime_error("outputType is NULL");
        
    if (!is_type(outputType))
        throw std::runtime_error(outputType->name + " is not a type");

    change_type(term, outputType);

    // Create state (if a state type is defined)
    Term* stateType = functionData.stateType;
    if (stateType != NULL && stateType != VOID_TYPE) {
        if (!is_type(stateType))
            throw std::runtime_error(stateType->name + " is not a type");
        term->state = create_value(NULL, stateType);
    }

    return term;
}
    
void set_input(Term* term, int index, Term* input)
{
    assert_good_pointer(term);

    term->inputs.setAt(index, input);
}

Term* create_duplicate(Branch* branch, Term* source, bool copyBranches)
{
    Term* term = create_term(branch, source->function, source->inputs);

    term->name = source->name;

    if (copyBranches)
        assign_value(source, term);
    else
        assign_value_but_dont_copy_inner_branch(source,term);

    duplicate_branch(source->properties, term->properties);

    if (source->state != NULL) {
        if (copyBranches || !has_inner_branch(source))
            assign_value(source->state, term->state);
    }
        
    return term;
}

Term* apply(Branch* branch, Term* function, RefList const& _inputs, std::string const& name)
{
    // Make a local copy of _inputs
    RefList inputs = _inputs;

    // Evaluate this function if needed
    if (function->needsUpdate)
        evaluate_term(function);

    // Check if 'function' is actually a type
    if (is_type(function))
    {
        if (inputs.count() != 0)
            throw std::runtime_error("Inputs in constructor function is not yet supported");

        function = get_value_function(function);
    }

    // If 'function' is a subroutine, then create a container for its state
    if (is_subroutine(function))
    {
        Term* stateContainer = create_value(branch, BRANCH_TYPE);
        source_set_hidden(stateContainer, true);
        inputs.prepend(stateContainer);
    }

    assert(is_function(function));

    // Create the term
    Term* result = create_term(branch, function, inputs);

    // Bind name, if given
    if (name != "" && branch != NULL)
        branch->bindName(result, name);

    return result;
}

Term* apply(Branch* branch, std::string const& functionName, RefList const& inputs, std::string const& name)
{
    Term* function = find_named(branch,functionName);
    if (function == NULL)
        throw std::runtime_error("function not found: "+functionName);

    return apply(branch, function, inputs, name);
}

Term* create_value(Branch* branch, Term* type, std::string const& name)
{
    assert(type != NULL);
    if (branch == NULL)
        assert(name == "");
    assert(is_type(type));

    Term *var_function = get_value_function(type);
    Term *term = create_term(branch, var_function, RefList());

    alloc_value(term);

    term->needsUpdate = false;
    term->stealingOk = false;

    if (name != "")
        branch->bindName(term, name);

    return term;
}

Term* create_value(Branch* branch, std::string const& typeName, std::string const& name)
{
    Term *type = NULL;

    type = find_named(branch, typeName);

    if (type == NULL)
        throw std::runtime_error(std::string("Couldn't find type: ")+typeName);

    return create_value(branch, type, name);
}

Term* import_value(Branch* branch, Term* type, void* initialValue, std::string const& name)
{
    assert(type != NULL);
    Term *var_function = get_value_function(type);
    Term *term = create_term(branch, var_function, RefList());

    term->value = initialValue;
    //term->ownsValue = false;
    term->stealingOk = false;

    if (name != "" && branch != NULL)
        branch->bindName(term, name);

    return term;
}

Term* import_value(Branch* branch, std::string const& typeName, void* initialValue, std::string const& name)
{
    Term* type = find_named(branch, typeName);

    if (type == NULL)
        throw std::runtime_error("Couldn't find type: "+typeName);

    return import_value(branch, type, initialValue, name);
}

Term* string_value(Branch* branch, std::string const& s, std::string const& name)
{
    Term* term = create_value(branch, STRING_TYPE, name);
    as_string(term) = s;
    return term;
}

Term* int_value(Branch* branch, int i, std::string const& name)
{
    Term* term = create_value(branch, INT_TYPE, name);
    as_int(term) = i;
    return term;
}

Term* float_value(Branch* branch, float f, std::string const& name)
{
    Term* term = create_value(branch, FLOAT_TYPE, name);
    as_float(term) = f;
    return term;
}

Term* bool_value(Branch* branch, bool b, std::string const& name)
{
    Term* term = create_value(branch, BOOL_TYPE, name);
    as_bool(term) = b;
    return term;
}

} // namespace circa
