// Copyright 2009 Andrew Fischer

#include "circa.h"

namespace circa {

Term* apply(Branch& branch, Term* function, RefList const& _inputs, std::string const& name)
{
    assert(function != NULL);
    if(!is_callable(function))
        throw std::runtime_error("Function "+function->name+" is not callable");

    // Check if 'function' is actually a type
    if (is_type(function))
        return create_value(branch, function, name);

    // Make a local copy of _inputs
    RefList inputs = _inputs;

    // Check to specialize function
    function = specialize_function(function, inputs);

    // Create the term
    Term* result = new Term();
    result->function = function;

    // If 'function' has hidden state, then create a container for that state, if needed
    if (is_function_stateful(function) && (inputs.length() < function_t::num_inputs(function)))
    {
        std::stringstream new_value_name;
        new_value_name << "#hidden_state";
        if (name != "") new_value_name << "_for_" << name;
        Term* stateContainer = create_value(branch, function_t::get_hidden_state_type(function),
                new_value_name.str());
        set_source_hidden(stateContainer, true);
        set_stateful(stateContainer, true);
        inputs.prepend(stateContainer);
    }

    // Add term to branch
    branch.append(result);

    if (name != "")
        branch.bindName(result, name);

    // Initialize inputs
    for (int i=0; i < inputs.length(); i++)
        set_input(result, i, inputs[i]);

    Term* outputType = function_t::get_output_type(function);

    assert(outputType != NULL);

    // Check if this function has a specializeType function
    // Side note: maybe we should do this step a different way.
    if (function_t::get_specialize_type(function) != NULL)
        outputType = function_t::get_specialize_type(function)(result);

    assert(outputType != NULL);
    assert(is_type(outputType));

    change_type(result, outputType);

    return result;
}

void set_input(Term* term, int index, Term* input)
{
    assert_good_pointer(term);
    term->inputs.setAt(index, input);

    // If this term is anonymous, check to update the 'statement' property. An anonymous
    // term can't be a statement if it's used as an input.
    if (input != NULL && input->name == "") {
        set_is_statement(input, false);
    }
}

void rewrite(Term* term, Term* function, RefList const& inputs)
{
    change_function(term, function);
    for (int i=0; i < inputs.length(); i++)
        set_input(term, i, inputs[i]);
    Term* outputType = function_t::get_output_type(function);

    if (function_t::get_specialize_type(function) != NULL)
        outputType = function_t::get_specialize_type(function)(term);

    change_type(term, outputType);
}

Term* create_duplicate(Branch& branch, Term* source, bool copyBranches)
{
    Term* term = apply(branch, source->function, source->inputs);
    change_type(term, source->type);

    term->name = source->name;

    if (source->value != NULL) {
        alloc_value(term);
        if (copyBranches || !is_branch(source))
            assign_value(source, term);
    }

    duplicate_branch(source->properties, term->properties);

    return term;
}

Term* apply(Branch& branch, std::string const& functionName, RefList const& inputs, std::string const& name)
{
    Term* function = find_named(branch, functionName);
    if (function == NULL)
        throw std::runtime_error("function not found: "+functionName);

    Term* result = apply(branch, function, inputs, name);
    result->stringProp("syntaxHints:functionName") = functionName;
    return result;
}

Term* create_value(Branch& branch, Term* type, std::string const& name)
{
    // This function is safe to call while bootstrapping.
    assert(type != NULL);
    assert(is_type(type));

    Term *term = new Term();
    branch.append(term);
    if (name != "")
        branch.bindName(term, name);

    term->function = VALUE_FUNC;
    term->type = type;
    change_type(term, type);
    alloc_value(term);

    return term;
}

Term* create_value(Branch& branch, std::string const& typeName, std::string const& name)
{
    Term* type = NULL;

    type = find_named(branch, typeName);

    if (type == NULL)
        throw std::runtime_error(std::string("Couldn't find type: ")+typeName);

    return create_value(branch, type, name);
}

Term* import_value(Branch& branch, Term* type, void* initialValue, std::string const& name)
{
    assert(type != NULL);
    Term *term = create_value(branch, type);

    term->value = initialValue;

    if (name != "")
        branch.bindName(term, name);

    return term;
}

Term* import_value(Branch& branch, std::string const& typeName, void* initialValue, std::string const& name)
{
    Term* type = find_named(branch, typeName);

    if (type == NULL)
        throw std::runtime_error("Couldn't find type: "+typeName);

    return import_value(branch, type, initialValue, name);
}

Term* string_value(Branch& branch, std::string const& s, std::string const& name)
{
    Term* term = create_value(branch, STRING_TYPE, name);
    as_string(term) = s;
    return term;
}

Term* int_value(Branch& branch, int i, std::string const& name)
{
    Term* term = create_value(branch, INT_TYPE, name);
    as_int(term) = i;
    return term;
}

Term* float_value(Branch& branch, float f, std::string const& name)
{
    Term* term = create_value(branch, FLOAT_TYPE, name);
    as_float(term) = f;
    return term;
}

Term* bool_value(Branch& branch, bool b, std::string const& name)
{
    Term* term = create_value(branch, BOOL_TYPE, name);
    as_bool(term) = b;
    return term;
}

Term* create_ref(Branch& branch, Term* ref, std::string const& name)
{
    Term* term = create_value(branch, REF_TYPE, name);
    as_ref(term) = ref;
    return term;
}

Branch& create_list(Branch& branch, std::string const& name)
{
    Term* term = create_value(branch, LIST_TYPE, name);
    return as_branch(term);
}

Branch& create_branch(Branch& owner, std::string const& name)
{
    Term* term = apply(owner, BRANCH_FUNC, RefList(), name);
    alloc_value(term);
    return as_branch(term);
}

Branch& create_namespace(Branch& branch, std::string const& name)
{
    return as_branch(create_value(branch, NAMESPACE_TYPE, name));
}

void rewrite_as_value(Branch& branch, int index, Term* type)
{
    while (index > branch.length())
        branch.append(NULL);

    if (index >= branch.length()) {
        create_value(branch, type);
    } else {
        Term* term = branch[index];

        change_function(term, VALUE_FUNC);
        change_type(term, type);
        term->inputs = RefList();
        alloc_value(term);
    }
}

} // namespace circa
