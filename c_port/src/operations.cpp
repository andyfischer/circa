
#include "common_headers.h"

#include "builtins.h"
#include "codeunit.h"
#include "errors.h"
#include "function.h"
#include "globals.h"
#include "operations.h"
#include "term.h"
#include "type.h"

Term* create_term(Term* function, TermList inputs)
{
    Term* term = new Term;
    initialize_term(term, function);
    set_inputs(term, inputs);
    return term;
}

void initialize_term(Term* term, Term* function)
{
    if (term == NULL)
        throw errors::InternalError("Term is NULL");

    if (function == NULL)
        throw errors::InternalError("Function is NULL");

    term->function = function;

    Term* outputType = as_function(function)->outputType;
    Term* stateType = as_function(function)->stateType;

    if (outputType == NULL)
        throw errors::InternalError("outputType is NULL");

    change_type(term, outputType);
}

Term* create_constant(Term* type)
{
    return create_term(get_const_function(type), TermList());
}

void change_type(Term* term, Term* type)
{
    if (term->value != NULL)
        throw errors::InternalError("value is not NULL in change_type (possible memory leak)");
    term->type = type;
    as_type(type)->alloc(type, term);
}

void specialize_type(Term* term, Term* type)
{
    if (term->type != BUILTIN_ANY_TYPE)
        throw errors::TypeError();

    change_type(term, type);
}

void set_input(Term* term, int index, Term* input)
{
    term->inputs.setAt(index, input);
}

void set_inputs(Term* term, TermList inputs)
{
    term->inputs = inputs;
}

Term* apply_function(Term* function, TermList inputs)
{
    // Check if 'function' is actually a type
    if (is_type(function))
    {
        return create_term(get_const_function(function), TermList());
    }

    // Create a term in the normal way
    return create_term(function, inputs);
}

Term* get_const_function(Term* type)
{
    Term* result = apply_function(GetGlobal("const-generator"), TermList(type));
    result->execute();
    return result;
}

Term* quick_create_type(CodeUnit* code, string name, Type::AllocFunc allocFunc,
        Function::ExecuteFunc toStringFunc)
{
    Term* typeTerm = create_constant(GetGlobal("Type"));
    as_type(typeTerm)->name = name;
    as_type(typeTerm)->alloc = allocFunc;
    code->bindName(typeTerm, name);

    // Create to-string function
    Term* toString = create_constant(GetGlobal("Function"));
    as_function(toString)->name = name + "-to-string";
    as_function(toString)->execute = toStringFunc;
    as_function(toString)->inputTypes.setAt(0, typeTerm);

    if (GetGlobal("string") == NULL)
        throw errors::InternalError("string type not defined");

    as_function(toString)->outputType = GetGlobal("string");
        
    as_type(typeTerm)->toString = toString;

    return typeTerm;
}

void transform_function_and_reeval(Term* term, Term* new_function)
{
    term->function = new_function;
    term->execute();
}
