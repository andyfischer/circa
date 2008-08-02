#include "common_headers.h"

#include "bootstrapping.h"
#include "branch.h"
#include "builtins.h"
#include "builtin_functions.h"
#include "function.h"
#include "operations.h"
#include "term.h"
#include "type.h"

namespace circa {

void to_string(Term* caller)
{
    Term* toString = caller->inputs[0]->getType()->toString;

    if (toString == NULL) {
        as_string(caller) = string("<") + as_type(caller->inputs[0]->type)->name + ">";
    } else {
        change_function(caller, toString);
        execute(caller);
    }
}

void print(Term* caller)
{
    std::cout << as_string(caller->inputs[0]) << std::endl;
}

void add(Term* caller)
{
    as_int(caller) = as_int(caller->inputs[0]) + as_int(caller->inputs[1]);
}

void mult(Term* caller)
{
    as_int(caller) = as_int(caller->inputs[0]) * as_int(caller->inputs[1]);
}

void string_concat(Term* caller)
{
    as_string(caller) = as_string(caller->inputs[0]) + as_string(caller->inputs[1]);
}

void create_list(Term* caller)
{
    as_list(caller)->clear();

    for (int i=0; i < caller->inputs.count(); i++) {
        as_list(caller)->append(caller->inputs[i]);
    }
}

void range(Term* caller)
{
    int max = as_int(caller->inputs[0]);

    as_list(caller)->clear();

    for (int i=0; i < max; i++) {
        as_list(caller)->append(constant_int(caller->owningBranch, i));
    }
}

void list_apply(Term* caller)
{
    as_function(caller->inputs[0]);
    TermList* list = as_list(caller->inputs[1]);

    as_list(caller)->clear();

    for (int i=0; i < list->count(); i++) {
        Term* result = apply_function(caller->owningBranch, caller->inputs[0], TermList(list->get(i)));
        execute(result);

        as_list(caller)->append(result);
    }
}

void this_branch(Term* caller)
{
    // TOFIX, this will have problems when memory management is implemented
    *as_list(caller) = caller->owningBranch->terms;
}

void initialize_builtin_functions(Branch* code)
{
    Term* int_t = get_global("int");
    Term* string_t = get_global("string");
    Term* any_t = get_global("any");
    Term* void_t = get_global("void");
    Term* function_t = get_global("Function");
    Term* list_t = get_global("List");

    quick_create_function(code, "to-string", to_string, TermList(any_t), string_t);
    quick_create_function(code, "add", add, TermList(int_t, int_t), int_t);
    quick_create_function(code, "mult", mult, TermList(int_t, int_t), int_t);
    quick_create_function(code, "concat", string_concat, TermList(string_t, string_t), string_t);
    quick_create_function(code, "print", print, TermList(string_t), void_t);
    quick_create_function(code, "list", create_list, TermList(any_t), list_t);
    quick_create_function(code, "range", range, TermList(int_t), list_t);
    quick_create_function(code, "list-apply", list_apply, TermList(function_t, list_t), list_t);
    quick_create_function(code, "this-branch", this_branch, TermList(), list_t);
}

} // namespace circa
