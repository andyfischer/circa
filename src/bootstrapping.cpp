
#include "common_headers.h"

#include "bootstrapping.h"
#include "branch.h"
#include "builtins.h"
#include "tokenizer.h"
#include "token_stream.h"
#include "operations.h"
#include "subroutine.h"

namespace token = circa::tokenizer;

namespace circa {

Term* quick_create_type(
        Branch* branch,
        string name,
        Type::AllocFunc allocFunc,
        Type::DeallocFunc deallocFunc,
        Type::CopyFunc copyFunc,
        Type::ToStringFunc toStringFunc)
{
    Term* typeTerm = create_constant(branch, get_global("Type"));
    as_type(typeTerm)->name = name;
    as_type(typeTerm)->alloc = allocFunc;
    as_type(typeTerm)->dealloc = deallocFunc;
    as_type(typeTerm)->copy = copyFunc;
    as_type(typeTerm)->toString = toStringFunc;
    branch->bindName(typeTerm, name);

    // Create to-string function
    /*
    if (toStringFunc != NULL) {
        Term* toString = create_constant(branch, get_global("Function"));
        as_function(toString)->name = name + "-to-string";
        as_function(toString)->execute = toStringFunc;
        as_function(toString)->inputTypes.setAt(0, typeTerm);

        if (get_global("string") == NULL)
            throw errors::InternalError("string type not defined");

        as_function(toString)->outputType = get_global("string");
        as_type(typeTerm)->toString = toString;
    }*/

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

void hosted_apply_function(Term* caller)
{
    recycle_value(caller->inputs[0], caller);
    Branch* branch = as_branch(caller);
    Term* function = caller->inputs[1];
    TermList* inputs = as_list(caller->inputs[2]);

    apply_function(branch, function, *inputs);
}

void hosted_to_string(Term* caller)
{
    as_string(caller) = caller->inputs[0]->toString();
}

void initialize_bootstrapped_code(Branch* kernel)
{
    Term* apply_function_f = quick_create_function(kernel, "apply-function",
        hosted_apply_function,
        TermList(BRANCH_TYPE, FUNCTION_TYPE, LIST_TYPE),
        BRANCH_TYPE);
    as_function(apply_function_f)->recycleInput = 0;

    quick_create_function(kernel, "to-string", hosted_to_string,
        TermList(ANY_TYPE), STRING_TYPE);

    // Parser
}

} // namespace circa
