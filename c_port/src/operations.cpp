
#include "common_headers.h"

#include "branch.h"
#include "builtins.h"
#include "errors.h"
#include "function.h"
#include "globals.h"
#include "operations.h"
#include "term.h"
#include "type.h"

Term* create_term(Branch* branch, Term* function, TermList inputs)
{
    if (!is_function(function))
        throw errors::InternalError("2nd arg to create_term must be a function");
    Term* term = new Term;
    initialize_term(term, function, inputs);
    
    return term;
}

void initialize_term(Term* term, Term* function, TermList inputs)
{
    if (term == NULL)
        throw errors::InternalError("Term is NULL");

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
        throw errors::InternalTypeError(term, BUILTIN_ANY_TYPE);

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

    Function* func = as_function(term->function);

    if (func == NULL)
        throw errors::InternalError("function is NULL");

    if (func->execute == NULL) {
        std::cout << "Error: no execute function for " << func->name << std::endl;
        return;
    }

    // Check if we need to recycle an input
    if (func->recycleInput != -1) {

        // Temporary measure: If this type has a copy function, copy. Otherwise
        // steal the value
        if (as_type(term->type)->copy == NULL)
            steal_value(term->inputs[0], term);
        else
            copy_term(term->inputs[0], term);
    }

    try {
        func->execute(term);
    }
    catch (errors::InternalError &err)
    {
        std::cout << "An internal error occured while executing " + func->name << std::endl;
        std::cout << err.message() << std::endl;
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

Term* get_const_function(Branch* branch, Term* type)
{
    Term* result = apply_function(branch, get_global("const-generator"), TermList(type));
    execute(result);
    return result;
}

Term* quick_create_type(Branch* branch, string name, Type::AllocFunc allocFunc,
        Function::ExecuteFunc toStringFunc, Type::CopyFunc copyFunc)
{
    Term* typeTerm = create_constant(branch, get_global("Type"));
    as_type(typeTerm)->name = name;
    as_type(typeTerm)->alloc = allocFunc;
    as_type(typeTerm)->copy = copyFunc;
    branch->bindName(typeTerm, name);

    // Create to-string function
    if (toStringFunc != NULL) {
        Term* toString = create_constant(branch, get_global("Function"));
        as_function(toString)->name = name + "-to-string";
        as_function(toString)->execute = toStringFunc;
        as_function(toString)->inputTypes.setAt(0, typeTerm);

        if (get_global("string") == NULL)
            throw errors::InternalError("string type not defined");

        as_function(toString)->outputType = get_global("string");
        as_type(typeTerm)->toString = toString;
    }

    return typeTerm;
}

Term* quick_create_function(Branch* branch, string name, Function::ExecuteFunc executeFunc,
        TermList inputTypes, Term* outputType)
{
    Term* term = create_constant(branch, get_global("Function"));
    Function* func = as_function(term);
    func->name = name;
    func->execute = executeFunc;
    func->inputTypes = inputTypes;
    func->outputType = outputType;
    branch->bindName(term, name);
	return term;
}

void change_function(Term* term, Term* new_function)
{
    if (new_function->type != BUILTIN_FUNCTION_TYPE)
        throw errors::InternalTypeError(new_function, BUILTIN_FUNCTION_TYPE);

    term->function = new_function;
}

void copy_term(Term* source, Term* dest)
{
    if (source->type != dest->type)
        throw errors::InternalTypeError(dest, source->type);

    Type::CopyFunc copy = as_type(source->type)->copy;

    if (copy == NULL)
        throw errors::InternalError(string("type ") + as_type(source->type)->name
                + " has no copy function");

    copy(source,dest);
}

void steal_value(Term* source, Term* dest)
{
    // Todo: delete the value at dest
    dest->value = source->value;
    source->value = NULL;
    source->needsUpdate = true;
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
