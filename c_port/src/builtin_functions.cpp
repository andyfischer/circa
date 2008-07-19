#include "common_headers.h"

#include "builtins.h"
#include "builtin_functions.h"
#include "codeunit.h"
#include "function.h"
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

void initialize_builtin_functions()
{
    CodeUnit* code = KERNEL;
}
