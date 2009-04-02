// Copyright 2009 Paul Hodge

#include "circa.h"

namespace circa {

Term* create_term(Branch* branch, Term* function, RefList const& inputs)
{
    assert_good_pointer(function);

    if (!is_function(function))
        throw std::runtime_error("in create_term, 2nd arg to create_term must be a function");

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
    if (stateType != NULL) {
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
        if (copyBranches && !has_inner_branch(source))
            assign_value(source->state, term->state);
    }
        
    return term;
}

Term* possibly_coerce_term(Branch* branch, Term* original, Term* expectedType)
{
    // (In the future, we will have more complicated coersion rules)
    
    // Ignore NULL
    if (original == NULL)
        return original;

    // Coerce from int to float
    if (original->type == INT_TYPE && expectedType == FLOAT_TYPE) {
        return apply(branch, INT_TO_FLOAT_FUNC, RefList(original));
    }

    return original;
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

    // If 'function' is not really a function, see if we can treat it like a function
    else if (!is_function(function)) {

        Type& type = as_type(function->type);

        if (!type.memberFunctions.contains(""))
            throw std::runtime_error(std::string("Term ") + function->toString()
                    + " is not a type, and has no default function");

        RefList memberFunctionInputs;
        memberFunctionInputs.append(function);
        memberFunctionInputs.appendAll(inputs);

        function = type.memberFunctions[""];
        inputs = memberFunctionInputs;
    }

    Function& functionData = as_function(function);

    // Possibly coerce inputs
    for (unsigned int i=0; i < inputs.count(); i++) {
        inputs.setAt(i, possibly_coerce_term(branch, inputs[i],
                functionData.inputType(i)));
    }

    // Create the term
    Term* result = create_term(branch, function, inputs);

    // Bind name, if given
    if (name != "" && branch != NULL)
        branch->bindName(result, name);

    return result;
}

Term* apply(Branch* branch, std::string const& functionName, RefList const& inputs)
{
    Term* function = find_named(branch,functionName);
    if (function == NULL)
        throw std::runtime_error("function not found: "+functionName);

    return apply(branch, function, inputs);
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


} // namespace circa
