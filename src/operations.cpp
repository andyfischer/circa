
#include "common_headers.h"

#include "branch.h"
#include "builtins.h"
#include "errors.h"
#include "function.h"
#include "operations.h"
#include "term.h"
#include "term_map.h"
#include "type.h"

namespace circa {

Term* create_term(Branch* branch, Term* function, TermList inputs)
{
    if (branch == NULL)
        throw errors::InternalError("in create_term, branch is NULL");
    if (!is_function(function))
        throw errors::InternalError("in create_term, 2nd arg to create_term must be a function");

    Term* term = new Term();
    term->owningBranch = branch;
    branch->terms.append(term);

    initialize_term(term, function, inputs);
    
    return term;
}

void initialize_term(Term* term, Term* function, TermList inputs)
{
    if (term == NULL)
        throw errors::InternalError("Term* is NULL");

    if (function == NULL)
        throw errors::InternalError("Function is NULL");

    term->function = function;
    Function* functionData = as_function(function);

    Term* outputType = functionData->outputType;
    Term* stateType = functionData->stateType;

    if (outputType == NULL)
        throw errors::InternalError("outputType is NULL");
        
    if (!is_type(outputType))
        throw errors::InternalError(outputType->findName() + " is not a type");

    if (stateType != NULL && !is_type(stateType))
        throw errors::InternalError(outputType->findName() + " is not a type");

    change_type(term, outputType);

    // Create state (if a state type is defined)
    if (stateType != NULL) 
        term->state = create_constant(term->owningBranch, stateType);
    else
        term->state = NULL;

    set_inputs(term, inputs);

    // Run the function's initialize (if it has one)
    if (functionData->initialize != NULL) {
        functionData->initialize(term);
    }

    // Add to the 'users' field of each input, and 'function'
    function->users.add(term);
    for (int index=0; index < inputs.count(); index++) {
        inputs[index]->users.add(term);
    }
}

void set_inputs(Term* term, TermList inputs)
{
    term->inputs = inputs;
}

Term* create_constant(Branch* branch, Term* type)
{
    return create_term(branch, get_const_function(branch, type), TermList());
}

void unsafe_change_type(Term* term, Term* type)
{
    if (term->value == NULL) {
        change_type(term, type);
        return;
    }

    term->type = type;
}

void change_type(Term* term, Term* type)
{
    if (term->value != NULL)
        throw errors::InternalError("value is not NULL in change_type (possible memory leak)");
    term->type = type;

    Type::AllocFunc alloc = as_type(type)->alloc;

    if (alloc == NULL) {
        throw errors::InternalError(string("type ") + as_type(type)->name
                + " has no alloc function");
    }

    alloc(term);
}

void specialize_type(Term* term, Term* type)
{
    if (term->type == type) {
        return;
    }

    if (term->type != ANY_TYPE)
        throw errors::TypeError(term, ANY_TYPE);

    change_type(term, type);
}

void set_input(Term* term, int index, Term* input)
{
    term->inputs.setAt(index, input);
}

void execute(Term* term)
{
    if (term->function == NULL)
        throw errors::InternalError("function term is NULL");

    // Check each input. Make sure:
    //  1) they are up-to-date
    //  2) they have a non-null value
    for (int inputIndex=0; inputIndex < term->inputs.count(); inputIndex++)
    {
        Term* input = term->inputs[inputIndex];

        if (input->needsUpdate)
            execute(input);
            
        if (input->value == NULL)
            throw errors::InternalError(string("Input named ") + input->findName() + " has NULL value.");
    }
    
    // Make sure we have an allocated value
    if (term->value == NULL) {
        // std::cout << "Reallocating term " << term->findName() << std::endl;
        as_type(term->type)->alloc(term);
    }    

    Function* func = as_function(term->function);

    if (func == NULL)
        throw errors::InternalError("function is NULL");

    if (func->execute == NULL) {
        std::cout << "Error: no execute function for " << func->name << std::endl;
        return;
    }

    try {
        func->execute(term);
        term->needsUpdate = false;
    }
    catch (errors::InternalError &err)
    {
        std::cout << "An internal error occured while executing " + func->name << std::endl;
        std::cout << err.message() << std::endl;
    }
}

void execute_branch(Branch* branch)
{
    int count = branch->terms.count();
    for (int index=0; index < count; index++) {
		Term* term = branch->terms[index];
        execute(term);
    }
}

Term* apply_function(Branch* branch, Term* function, TermList inputs)
{
    // Check if 'function' is actually a type
    if (is_type(function))
    {
        return create_term(branch, get_const_function(branch, function), TermList());
    }

    // Create a term in the normal way
    return create_term(branch, function, inputs);
}

Term* exec_function(Branch* branch, Term* function, TermList inputs)
{
    Term* result = apply_function(branch, function, inputs);
    execute(result);
    return result;
}

Term* get_const_function(Branch* branch, Term* type)
{
    Term* result = apply_function(branch, get_global("const-generator"), TermList(type));
    execute(result);
    return result;
}

bool is_constant(Term* term)
{
    return term->function->function == CONST_GENERATOR;
}

void change_function(Term* term, Term* new_function)
{
    if (new_function->type != FUNCTION_TYPE)
        throw errors::TypeError(new_function, FUNCTION_TYPE);

    term->function = new_function;
}

void recycle_value(Term* source, Term* dest)
{
    // Don't steal if the term has multiple users
    bool steal = (source->users.count() > 1);

    // Temp: always try to steal
    steal_value(source, dest);
}

void copy_value(Term* source, Term* dest)
{
    if (source->type != dest->type)
        throw errors::TypeError(dest, source->type);

    Type::CopyFunc copy = as_type(source->type)->copy;

    if (copy == NULL)
        throw errors::InternalError(string("type ") + as_type(source->type)->name
                + " has no copy function");

    copy(source,dest);
}

void steal_value(Term* source, Term* dest)
{
    if (source->type != dest->type)
        throw errors::TypeError(dest, source->type);

    // In some situations, ignore their request to steal

    // Don't steal from constant terms
    if (is_constant(source)) {
        copy_value(source, dest);
        return;
    }

    // if 'dest' has a value, delete it
    if (dest->value != NULL) {
        if (as_type(dest->type)->dealloc != NULL) {
            as_type(dest->type)->dealloc(dest);
            dest->value = NULL;
        }
        else {
            std::cout << "Warning: type " << as_type(dest->type)->name
                << " needs a dealloc function" << std::endl;
        }
    }

    dest->value = source->value;
    source->value = NULL;
    source->needsUpdate = true;
}

void duplicate_branch(Branch* source, Branch* dest)
{
    TermMap newTermMap;

    // Duplicate every term
    for (int index=0; index < source->terms.count(); index++) {
        Term* source_term = source->terms[index];

        Term* dest_term = create_term(dest, source_term->function, source_term->inputs);
        newTermMap[source_term] = dest_term;

        copy_value(source_term, dest_term);
    }

    // Remap terms
    for (int index=0; index < dest->terms.count(); index++) {
        Term* term = dest->terms[index];
        term->inputs.remap(newTermMap);
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

Term* find_named(Branch* branch, std::string name)
{
    if (branch->containsName(name))
        return branch->getNamed(name);

    return get_global(name);
}

Term* constant_string(Branch* branch, std::string s)
{
    Term* term = apply_function(branch, STRING_TYPE, TermList());
    as_string(term) = s;
    return term;
}

Term* constant_int(Branch* branch, int i)
{
    Term* term = apply_function(branch, INT_TYPE, TermList());
    as_int(term) = i;
    return term;
}

Term* constant_float(Branch* branch, float f)
{
    Term* term = apply_function(branch, FLOAT_TYPE, TermList());
    as_float(term) = f;
    return term;
}

Term* constant_list(Branch* branch, TermList list)
{
    Term* term = apply_function(branch, LIST_TYPE, TermList());
    *as_list(term) = list;
    return term;
}

} // namespace circa
