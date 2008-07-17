#include "common_headers.h"

#include "bootstrap.h"
#include "builtin_functions.h"
#include "codeunit.h"
#include "exec_context.h"
#include "function.h"
#include "object.h"
#include "primitive_types.h"
#include "term.h"
#include "type.h"

void to_string(ExecContext* cxt)
{
    Term* type = cxt->inputTerm(0)->getType();
    Term* toString = type->outputValue->asType()->toString;
    CircaObject* result = CaCode_executeFunction(toString, TermList(cxt->inputTerm(0)));
    cxt->result()->asString()->value = result->asString()->value;
}

void initialize_builtin_functions()
{
    CodeUnit* code = KERNEL;
}
