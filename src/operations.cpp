// Copyright 2008 Andrew Fischer

#include "common_headers.h"

#include "branch.h"
#include "builtins.h"
#include "errors.h"
#include "function.h"
#include "list.h"
#include "operations.h"
#include "parser.h"
#include "ref_list.h"
#include "ref_map.h"
#include "term.h"
#include "type.h"
#include "values.h"

namespace circa {

Term* create_term(Branch* branch, Term* function, ReferenceList inputs)
{
    if (branch == NULL)
        throw std::runtime_error("in create_term, branch is NULL");
    if (!is_function(function))
        throw std::runtime_error("in create_term, 2nd arg to create_term must be a function");

    Term* term = new Term();

    if (branch != NULL)
        branch->append(term);

    initialize_term(term, function, inputs);
    
    return term;
}

void initialize_term(Term* term, Term* function, ReferenceList inputs)
{
    if (term == NULL)
        throw errors::InternalError("Term* is NULL");

    if (function == NULL)
        throw errors::InternalError("Function is NULL");

    term->function = function;
    Function& functionData = as_function(function);

    Term* outputType = functionData.outputType;
    Term* stateType = functionData.stateType;

    if (outputType == NULL)
        throw errors::InternalError("outputType is NULL");
        
    if (!is_type(outputType))
        throw errors::InternalError(outputType->findName() + " is not a type");

    if (stateType != NULL && !is_type(stateType))
        throw errors::InternalError(outputType->findName() + " is not a type");

    change_type(term, outputType);

    // Create state (if a state type is defined)
    if (stateType != NULL) {
        term->state = create_constant(NULL, stateType);
    }
    else
        term->state = NULL;

    set_inputs(term, inputs);

    // Run the function's initialize (if it has one)
    if (functionData.initialize != NULL) {
        functionData.initialize(term);
    }

    // Add to the 'users' field of each input, and 'function'
    /*function->users.add(term);
    for (unsigned int index=0; index < inputs.count(); index++) {

        if (inputs[index] == NULL)
            continue;

        inputs[index]->users.add(term);
    }*/
}

void set_inputs(Term* term, ReferenceList inputs)
{
    term->inputs = inputs;
}

Term* create_constant(Branch* branch, Term* type)
{
    Term *term = create_term(branch, get_const_function(branch, type), ReferenceList());
    term->stealingOk = false;
    return term;
}

void set_input(Term* term, int index, Term* input)
{
    term->inputs.setAt(index, input);
}

Term* apply_function(Branch* branch, Term* function, ReferenceList inputs)
{
    if (function->needsUpdate)
        function->eval();

    // Check if 'function' is actually a type
    if (is_type(function))
    {
        if (inputs.count() != 0)
            throw errors::InternalError("Multiple inputs in constructor not supported");

        return create_term(branch, get_const_function(branch, function), inputs);
    }

    // If 'function' is not really a function, see if we can treat it like a function
    if (!is_function(function)) {

        Type* type = as_type(function->type);

        if (!type->memberFunctions.contains(""))
            throw errors::InternalError(std::string("Term ") + function->toString()
                    + " is not a type, and has no default function");

        ReferenceList memberFunctionInputs;
        memberFunctionInputs.append(function);
        memberFunctionInputs.appendAll(inputs);

        return create_term(branch, type->memberFunctions[""], memberFunctionInputs);
    }

    // Create a term in the normal way
    return create_term(branch, function, inputs);
}

Term* eval_function(Branch* branch, Term* function, ReferenceList inputs)
{
    Term* result = apply_function(branch, function, inputs);
    result->eval();
    return result;
}

Term* get_const_function(Branch* branch, Term* type)
{
    Term* result = apply_function(branch, CONST_GENERATOR, ReferenceList(type));
    result->eval();
    return result;
}

bool is_constant(Term* term)
{
    return term->function->function == CONST_GENERATOR;
}

void change_function(Term* term, Term* new_function)
{
    assert_good(term);

    if (new_function->type != FUNCTION_TYPE)
        throw errors::TypeError(new_function, FUNCTION_TYPE);

    term->function = new_function;
}

void remap_pointers(Term* term, ReferenceMap &map)
{
    assert_good(term);

    term->inputs.remapPointers(map);
    term->function = map.getRemapped(term->function);

    if (as_type(term->type)->remapPointers != NULL)
        as_type(term->type)->remapPointers(term, map);
}
void remap_pointers(Term* term, Term* original, Term* replacement)
{
    ReferenceMap map;
    map[original] = replacement;
    remap_pointers(term, map);
}

void duplicate_branch(Branch* source, Branch* dest)
{
    ReferenceMap newTermMap;

    // Duplicate every term
    for (int index=0; index < source->numTerms(); index++) {
        Term* source_term = source->get(index);

        Term* dest_term = create_term(dest, source_term->function, source_term->inputs);
        newTermMap[source_term] = dest_term;

        duplicate_value(source_term, dest_term);
    }

    // Remap terms
    for (int index=0; index < dest->numTerms(); index++) {
        Term* term = dest->get(index);
        term->inputs.remapPointers(newTermMap);
        if (as_type(term->type)->remapPointers != NULL)
            as_type(term->type)->remapPointers(term, newTermMap);
    }

    // Copy names
    TermNamespace::StringToTermMap::iterator it;
    for (it = source->names.begin(); it != source->names.end(); ++it) {
        std::string name = it->first;
        Term* original_term = it->second;
        dest->bindName(newTermMap.getRemapped(original_term), name);
    }
}

Term* constant_string(Branch* branch, std::string const& s, std::string const& name)
{
    Term* term = apply_function(branch, STRING_TYPE, ReferenceList());
    as_string(term) = s;
    if (name != "")
        branch->bindName(term, name);
    return term;
}

Term* constant_int(Branch* branch, int i, std::string const& name)
{
    Term* term = apply_function(branch, INT_TYPE, ReferenceList());
    as_int(term) = i;
    if (name != "")
        branch->bindName(term, name);
    return term;
}

Term* constant_float(Branch* branch, float f, std::string const& name)
{
    Term* term = apply_function(branch, FLOAT_TYPE, ReferenceList());
    as_float(term) = f;
    if (name != "")
        branch->bindName(term, name);
    return term;
}

Term* constant_list(Branch* branch, ReferenceList list, std::string const& name)
{
    Term* term = apply_function(branch, LIST_TYPE, ReferenceList());
    // FIXME as_list(term) = list;
    if (name != "")
        branch->bindName(term, name);
    return term;
}

} // namespace circa
