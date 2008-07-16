
#include "common_headers.h"

#include "exec_context.h"
#include "term.h"
#include "object.h"

Term*
ExecContext::inputTerm(int index)
{
    return target->inputs[index];
}

CircaObject*
ExecContext::input(int index)
{
    return target->inputs[index]->outputValue;
}

CircaObject*
ExecContext::result()
{
    return target->outputValue;
}
