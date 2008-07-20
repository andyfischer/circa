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
    /*
    Term* type = caller->inputs[0]->getType();
    Term* tostring = type->outputValue->asType()->toString;
    CircaObject* result = CaCode_executeFunction(toString, TermList(cxt->inputTerm(0)));
    cxt->result()->asString()->value = result->asString()->value;*/
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
}

void add(Term* caller)
{
    as_int(caller) = as_int(caller->inputs[0]) + as_int(caller->inputs[1]);
}

void initialize_builtin_functions()
{
    CodeUnit* code = KERNEL;

    Term* int_t = GetGlobal("int");
    Term* float_t = GetGlobal("int");

    create_func(code, "add", add, TermList(int_t, int_t), int_t);
}
