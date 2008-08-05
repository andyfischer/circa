
#include "common_headers.h"

#include "branch.h"
#include "builtins.h"
#include "errors.h"
#include "function.h"
#include "logging.h"
#include "operations.h"
#include "term.h"
#include "term_map.h"
#include "type.h"

namespace circa {

Term* create_term(Branch* branch, Term* function, TermList inputs)
{
    if (!is_function(function))
        throw errors::InternalError("2nd arg to create_term must be a function");
    Term* term = new Term();
    initialize_term(term, function, inputs);

    // Add to branch
    branch->terms.append(term);
    term->owningBranch = branch;
    
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
    // Term* stateType = functionData->stateType;

    if (outputType == NULL)
        throw errors::InternalError("outputType is NULL");

    change_type(term, outputType);

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

void change_type(Term* term, Term* type)
{
    if (term->value != NULL)
        throw errors::InternalError("value is not NULL in change_type (possible memory leak)");
    term->type = type;

    Type::AllocFunc alloc = as_type(type)->alloc;

    if (alloc == NULL)
        throw errors::InternalError(string("type ") + as_type(type)->name
                + " has no alloc function");

    alloc(term);
}

void specialize_type(Term* term, Term* type)
{
    if (term->type != BUILTIN_ANY_TYPE)
        throw errors::TypeError(term, BUILTIN_ANY_TYPE);

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
        std::cout << "Reallocating term " << term->findName() << std::endl;
        as_type(term->type)->alloc(term);
    }    

    Function* func = as_function(term->function);

    if (func == NULL)
        throw errors::InternalError("function is NULL");

    if (func->execute == NULL) {
        std::cout << "Error: no execute function for " << func->name << std::endl;
        return;
    }

    // Check if we should recycle an input
    if (func->recycleInput != -1) {

        Term* recycleTerm = term->inputs[func->recycleInput];
        bool steal = true;

        // Don't steal if the term has multiple users
        steal = steal && (recycleTerm->users.count() > 1);

        if (steal)
            steal_value(recycleTerm, term);
        else
            copy_value(recycleTerm, term);
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
    if (new_function->type != BUILTIN_FUNCTION_TYPE)
        throw errors::TypeError(new_function, BUILTIN_FUNCTION_TYPE);

    term->function = new_function;
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
    // In some situations, ignore their request to steal

    // Don't steal from constant terms
    if (is_constant(source)) {
        copy_value(source, dest);
        return;
    }

    std::cout << "Stealing value from " << dest->findName() << ", " << (int) dest << std::endl;
    
    // if 'dest' has a value, delete it
    if (dest->value != NULL) {
        if (as_type(dest->type)->dealloc != NULL)
            as_type(dest->type)->dealloc(dest);
        else
            std::cout << "Warning: type " << as_type(dest->type)->name
                << " needs a dealloc function" << std::endl;
    }

    dest->value = source->value;
    source->value = NULL;
    source->needsUpdate = true;
}

void duplicate_branch(Term* source, Term* dest)
{
    Branch* source_branch = as_branch(source);
    Branch* dest_branch = as_branch(dest);

    TermMap newTermMap;

    // Duplicate every term
    for (int index=0; index < source_branch->terms.count(); index++) {
        Term* source_term = source_branch->terms[index];

        Term* dest_term = create_term(dest_branch, source_term->function, source_term->inputs);
        newTermMap[source_term] = dest_term;
    }

    // Remap terms
    for (int index=0; index < dest_branch->terms.count(); index++) {
        Term* term = dest_branch->terms[index];
        term->inputs.remap(newTermMap);
        if (as_type(term->type)->remapPointers != NULL)
            as_type(term->type)->remapPointers(term, newTermMap);
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
    Term* term = apply_function(branch, get_global("string"), TermList());
    as_string(term) = s;
    return term;
}

Term* constant_int(Branch* branch, int i)
{
    Term* term = apply_function(branch, get_global("int"), TermList());
    as_int(term) = i;
    return term;
}

Term* constant_list(Branch* branch, TermList list)
{
    Term* term = apply_function(branch, get_global("List"), TermList());
    *as_list(term) = list;
    return term;
}

} // namespace circa
