
#include "common_headers.h"

#include "builtins.h"
#include "codeunit.h"
#include "errors.h"
#include "function.h"
#include "globals.h"
#include "operations.h"
#include "term.h"
#include "type.h"

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

void set_inputs(Term* term, TermList inputs)
{
    term->inputs = inputs;
}

Term* quick_create_type(CodeUnit* code, string name, Type::AllocFunc allocFunc,
        Function::ExecuteFunc toStringFunc)
{
    Term* typeTerm = code->createConstant(GetGlobal("Type"), NULL);
    as_type(typeTerm)->name = name;
    as_type(typeTerm)->alloc = allocFunc;
    code->bindName(typeTerm, name);

    // Create to-string function
    Term* toString = code->createConstant(GetGlobal("Function"), NULL);
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
