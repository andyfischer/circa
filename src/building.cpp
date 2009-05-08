// Copyright 2009 Andrew Fischer

#include "circa.h"

namespace circa {

Term* apply(Branch* branch, Term* function, RefList const& _inputs, std::string const& name)
{
    // Make a local copy of _inputs
    RefList inputs = _inputs;

    // Evaluate this function if needed
    if (function->needsUpdate)
        evaluate_term(function);

    // Check if 'function' is actually a type
    Term* valueType = NULL;
    if (is_type(function))
    {
        if (inputs.count() != 0)
            throw std::runtime_error("Inputs in constructor function is not yet supported");

        valueType = function;
        function = VALUE_FUNC;
    }

    assert(is_function(function));

    Function& func = as_function(function);

    // If 'function' has hidden state, then create a container for that state, if needed
    if (has_hidden_state(func) && ((int) inputs.count() < func.numInputs()))
    {
        Term* stateContainer = create_value(branch, func.hiddenStateType);
        source_set_hidden(stateContainer, true);
        set_stateful(stateContainer, true);
        inputs.prepend(stateContainer);
    }

    // Create the term
    Term* result = new Term();

    if (branch != NULL) {
        branch->append(result);

        if (name != "")
            branch->bindName(result, name);
    }

    result->function = function;
    result->needsUpdate = true;

    // Initialize inputs
    for (unsigned int i=0; i < inputs.count(); i++)
        set_input(result, i, inputs[i]);

    Term* outputType = func.outputType;

    // Check if this function has a specializeType function
    // Side note: maybe we should do this step later. Doing it here means that we can only
    // specialize on inputs, but it might be cool to specialize on state too.
    if (func.specializeType != NULL)
        outputType = func.specializeType(result);

    // If we were called with a type, then use that type
    if (valueType != NULL)
        outputType = valueType;

    assert(outputType != NULL);
    assert(is_type(outputType));

    change_type(result, outputType);

    // Temporary hack
    if (function == BRANCH_FUNC)
        as_branch(result).outerScope = branch;

    return result;
}

void set_input(Term* term, int index, Term* input)
{
    assert_good_pointer(term);

    term->inputs.setAt(index, input);
}

Term* create_duplicate(Branch* branch, Term* source, bool copyBranches)
{
    Term* term = apply(branch, source->function, source->inputs);
    change_type(term, source->type);

    term->name = source->name;

    if (copyBranches)
        assign_value(source, term);
    else
        assign_value_but_dont_copy_inner_branch(source,term);

    duplicate_branch(source->properties, term->properties);

    return term;
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
    assert(is_type(type));

    Term *term = apply(branch, VALUE_FUNC, RefList(), name);
    change_type(term, type);

    alloc_value(term);

    term->needsUpdate = false;
    term->stealingOk = false;

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
    Term *term = create_value(branch, type);

    term->value = initialValue;
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

void rewrite_as_value(Branch& branch, int index, Term* type)
{
    while (index > branch.length())
        branch.append(NULL);

    if (index >= branch.length()) {
        create_value(&branch, type);
    } else {
        Term* term = branch[index];

        change_function(term, VALUE_FUNC);
        change_type(term, type);
        term->inputs = RefList();
        alloc_value(term);
    }
}

} // namespace circa
