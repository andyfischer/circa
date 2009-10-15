// Copyright (c) 2007-2009 Andrew Fischer. All rights reserved.

#include "circa.h"

namespace circa {

Term* apply(Branch& branch, Term* function, RefList const& _inputs, std::string const& name)
{
    assert(function != NULL);
    if(!is_callable(function))
        throw std::runtime_error("Function "+function->name+" is not callable");

    // Check if 'function' is actually a type
    if (is_type(function)) {
        Term* result = create_value(branch, function, name);
        result->boolProp("constructor") = true;
        return result;
    }

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
        Term* stateContainer = create_stateful_value(branch, function_t::get_hidden_state_type(function),
                new_value_name.str());
        set_source_hidden(stateContainer, true);
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
}

Term* create_duplicate(Branch& branch, Term* source, std::string const& name, bool copyBranches)
{
    Term* term = apply(branch, source->function, source->inputs, name);
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
        throw std::runtime_error("Couldn't find type: "+typeName);

    return create_value(branch, type, name);
}

Term* create_stateful_value(Branch& branch, Term* type, std::string const& name)
{
    Term* t = create_value(branch, type, name);
    t->function = STATEFUL_VALUE_FUNC;
    return t;
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

Term* ref_value(Branch& branch, Term* ref, std::string const& name)
{
    Term* term = create_value(branch, REF_TYPE, name);
    as_ref(term) = ref;
    return term;
}
Term* create_void(Branch& branch, std::string const& name)
{
    return create_value(branch, VOID_TYPE, name);
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

Term* create_type(Branch& branch, std::string name)
{
    Term* term = create_value(branch, TYPE_TYPE);

    if (name != "") {
        type_t::get_name(term) = name;
        branch.bindName(term, name);
    }

    return term;
}

Term* create_empty_type(Branch& branch, std::string name)
{
    Term* type = create_type(branch, name);
    initialize_empty_type(type);
    return type;
}

Term* create_compound_type(Branch& branch, std::string const& name)
{
    Term* term = create_type(branch, name);
    initialize_compound_type(term);
    return term;
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

void resize_list(Branch& list, int numElements, Term* type)
{
    // Add terms if necessary
    for (int i=list.length(); i < numElements; i++)
        create_value(list, type);

    // Remove terms if necessary
    bool anyRemoved = false;
    for (int i=numElements; i < list.length(); i++) {
        list[i] = NULL;
        anyRemoved = true;
    }

    if (anyRemoved)
        list.removeNulls();
}

void set_step(Term* term, float step)
{
    term->floatProp("step") = step;
}

float get_step(Term* term)
{
    return term->floatPropOptional("step", 1.0);
}

} // namespace circa
