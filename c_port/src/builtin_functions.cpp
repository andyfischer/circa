#include "common_headers.h"

#include "builtins.h"
#include "builtin_functions.h"
#include "codeunit.h"
#include "function.h"
#include "globals.h"
#include "term.h"
#include "type.h"

void to_string(Term* caller)
{
    Term* toString = caller->inputs[0]->getType()->toString;

    if (toString == NULL) {
        as_string(caller) = string("<") + as_type(caller->type)->name + ">";
    } else {
        transform_function_and_reeval(caller, toString);
    }
}

void create_func(CodeUnit* code, string name, void (*execute)(Term*),
        TermList inputTypes, Term* outputType)
{
    Term* term = code->createConstant(GetGlobal("Function"), NULL);
    Function* func = as_function(term);
    func->name = name;
    func->execute = execute;
    func->inputTypes = inputTypes;
    func->outputType = outputType;
    code->bindName(term, name);
}

void add(Term* caller)
{
    as_int(caller) = as_int(caller->inputs[0]) + as_int(caller->inputs[1]);
}

void create_builtin_functions()
{
    CodeUnit* code = KERNEL;

    Term* int_t = GetGlobal("int");
    Term* float_t = GetGlobal("int");
    Term* string_t = GetGlobal("string");
    Term* any_t = GetGlobal("any");

    create_func(code, "to-string", to_string, TermList(any_t), string_t);
    create_func(code, "add", add, TermList(int_t, int_t), int_t);
}

