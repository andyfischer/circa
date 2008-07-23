#include "common_headers.h"

#include "builtins.h"
#include "builtin_functions.h"
#include "codeunit.h"
#include "function.h"
#include "globals.h"
#include "operations.h"
#include "term.h"
#include "type.h"

void to_string(Term* caller)
{
    Term* toString = caller->inputs[0]->getType()->toString;

    if (toString == NULL) {
        // std::cout << "Warning: toString is NULL" << std::endl;
        as_string(caller) = string("<") + as_type(caller->inputs[0]->type)->name + ">";
    } else {
        change_function(caller, toString);
        execute(caller);
    }
}


void add(Term* caller)
{
    as_int(caller) = as_int(caller->inputs[0]) + as_int(caller->inputs[1]);
}

void initialize_builtin_functions(CodeUnit* code)
{
    Term* int_t = GetGlobal("int");
    //Term* float_t = GetGlobal("int");
    Term* string_t = GetGlobal("string");
    Term* any_t = GetGlobal("any");

    quick_create_function(code, "to-string", to_string, TermList(any_t), string_t);
    quick_create_function(code, "add", add, TermList(int_t, int_t), int_t);
}

